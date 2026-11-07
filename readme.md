# Hướng Dẫn Cấu Hình Kiến Trúc & Vận Hành OTA (STM32F411)

---

## 1. Tổng Quan & Cách Sử Dụng `app1` và `app2`

Hệ thống hoạt động theo mô hình Dual-Slot nạp sẵn trên 2 vi điều khiển (hoặc 2 phân vùng):

| Thuộc tính | `app1` (Slave / Node Nhận) | `app2` (Master / Node Mồi - Seed Node) |
| :--- | :--- | :--- |
| **Phân vùng Flash thực thi** | **Slot 1 (Sector 5 - `0x08020000`)** | **Slot 2 (Sector 6 - `0x08040000`)** |
| **Cấu hình `custom.ld`** | `REGION_ALIAS("FLASH_APP", FLASH_APP1);` | `REGION_ALIAS("FLASH_APP", FLASH_APP2);` |
| **Ưu tiên Boot (`bootloader.c`)** | Ưu tiên Slot 1 khi trạng thái `STATE_STABLE` | Ưu tiên Slot 2 khi trạng thái `STATE_STABLE` |
| **Vai trò vận hành** | Nhận bản cập nhật qua UART2 và nạp vào Flash | Chủ động đọc firmware từ Slot 2 và phát OTA sang Slave |
| **Tập lệnh CLI (UART1)** | Lệnh cơ bản (log, test, flash info...) | Bổ sung thêm lệnh **`ota_start`** |

### Cách chạy thực nghiệm cập nhật OTA giữa 2 mạch:
1. **Đấu nối phần cứng:**
   - **Cổng giao tiếp OTA (UART2):** Nối chéo chân giữa 2 board STM32:
     - `USART2_TX (PA2 - Board 2)` ↔ `USART2_RX (PA3 - Board 1)`
     - `USART2_RX (PA3 - Board 2)` ↔ `USART2_TX (PA2 - Board 1)`
     - Nối chung đường `GND`.
   - **Cổng Debug / CLI (UART1):** Nối `PA9 (TX)` và `PA10 (RX)` của từng board lên module USB-to-UART máy tính để xem log.
2. **Quy trình kích hoạt:**
   - Mở Serial Terminal ở tốc độ `115200 bps` trên cổng UART1 của **Board 2 (`app2`)**.
   - Gõ lệnh:
     ```bash
     ota_start
     ```
   - Quá trình Stop-and-Wait sẽ tự động thực hiện: Unlock & Erase Sector 5 trên Board 1 → Truyền từng khối data 128B → Kiểm tra mã băm SHA-256 → Gửi lệnh Reboot Board 1 sang firmware mới.

---

## 2. Lưu Ý Quan Trọng Khi Cấu Hình CubeMX

> [!WARNING]
> **Tuyệt đối KHÔNG bật `#define USER_VECT_TAB_ADDRESS`** trong mã nguồn vì hàm SystemInit sẽ ghi đè thanh ghi VTOR về mặc định của Flash, làm mất định tuyến bảng ngắt của hệ thống.

- **Tab "Code Generator":** Bật tùy chọn `Generate peripheral initialization as a pair of '.c/.h' files per peripheral`.
- **Tab "Project":** Bật tùy chọn `Do not generate the main()` (vì hàm `main` được quản lý tại `<Thư mục project>/Core/user/app/start/appstart.c`).
- CubeMX không đóng vai trò sinh mã luồng FreeRTOS (không tạo task trên giao diện đồ họa).

---

## 3. Các Bước Cấu Hình STM32CubeMX

### 3.1. Pinout & Configuration
- **System Core:**
  - **RCC** > Mode > HSE: `Crystal/Ceramic Resonator`
  - **SYS** > Mode > Debug: `Serial Wire`
  - **SYS** > Timebase Source: `TIM1`
- **Connectivity:**
  - **USART1** > Mode: `Asynchronous`
    - DMA Settings > DMA Request: thêm `USART1_RX` và `USART1_TX`
  - **USART2** > Mode: `Asynchronous`
    - DMA Settings > DMA Request: thêm `USART2_RX` và `USART2_TX`
- **Computing:**
  - **CRC** > `Activate`
- **Middleware and Software Packs:**
  - **FREERTOS** > Mode > Interface: `CMSIS_V2`

### 3.2. Clock Configuration
- **PLL Source Mux:** `HSE`
- **HCLK:** `100 MHz`

### 3.3. Project Manager
- **Toolchain / IDE:** `CMake`
- **Code Generator:**
  - Chọn: `Generate peripheral initialization as a pair of '.c/.h' files per peripheral`
- **Advanced Settings:**
  - **Register CallBack** > UART: `ENABLE`

---

## 4. Tích Hợp Thư Mục Mã Nguồn & Cấu Hình Build

### 4.1. Sắp xếp thư mục
- Tạo code từ CubeMX.
- Copy thư mục `user` vào `<tên dự án>/Core` (đường dẫn đích sẽ là `<Thư mục dự án>/Core/user`).

### 4.2. Cấu hình `CMakeLists.txt`
1. Thêm cờ tối ưu hóa sau dòng `enable_language(C ASM)`:
```cmake
set(CMAKE_C_FLAGS_DEBUG "-Os -g3" CACHE STRING "" FORCE)
add_compile_options(-Os)
```

2. Thêm thư mục `Core/user` và liên kết thư viện vào cuối file:
```cmake
# Add custom User
add_subdirectory(Core/user)
target_link_libraries(${CMAKE_PROJECT_NAME}
    service_lib
    app_lib
)
```

### 4.3. Cấu hình Linker Script
Tìm đến dòng sau trong file `<Thư mục dự án>/cmake/gcc-arm-none-eabi.cmake`:
```cmake
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -T \"${CMAKE_SOURCE_DIR}/STM32F411xx_FLASH.ld\"")
```
Sửa thành:
```cmake
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -T \"${CMAKE_SOURCE_DIR}/Core/user/custom.ld\"")
```

### 4.4. Khởi tạo Ứng dụng trong `main.c`
1. Thêm các thư viện ở đầu file `main.c`:
```c
#include "app/start/appstart.h"
#include "app/start/command.h"
```

2. Ngay sau dòng `osKernelInitialize();`, thêm mã khởi tạo:
```c
if (appstart_Init(&huart1, &huart2, &hcrc, command_list) != appstart_OK) {
    // Nếu khởi tạo thất bại, dừng chương trình để bảo vệ hệ thống
    while (1);
}
```

