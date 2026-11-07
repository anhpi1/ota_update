Trong "cubemx" tại tap "code gennerator" bật tùy chọn "Generate peripheral initialization as a pair of '.c/.h' files per peripheral"
Trong "cubemx" tại tap "project" bật tùy chọn "Do not generate the main()" vì hàm main sẽ ở "<Thư mục project>/core/user/app/start/appstart.c"
"cubemx" sẽ không đóng vai trò sinh code ví dụ như tạo tas
#define USER_VECT_TAB_ADDRESS tuyệt đối không được bật cái này vì sys init sẽ ghi đè thanh ghi VTOR về lỗ


hướng dẫn cấu hình cubemx

Pinout & Configuration
	System Core > mode > RCC > HSE > Crystal/Ceramic Resonator
	SYS > mode > Debug 	> Serial Wire
						> Timebase Source > TIM1
	Connectivity > mode > USART1 > Mode > Asynchronous
								 > Configuration > DMA Settings > DMA Request 	> USART1_RX 
								 												> USART1_TX 
	Connectivity > mode > USART2 > Mode > Asynchronous
								 > Configuration > DMA Settings > DMA Request 	> USART1_RX 
								 												> USART1_TX 
	Computing > mode > CRC > Activate
	Middleware and Software Packs > FREERTOS > mode > Interface > CMSIS_V2
Clock Configuration 
	PLL Source Mux > HSE
	HCLK > 100
Project Manager
	Toolchain / IDE > CMake
	Code Generator > Generated > files > Generate peripheral initialization as a pair of '.c/.h' files per peripheral
	Advanced Settings > Register CallBack > UART > ENABLE

Tạo code 

copy các thư mục trong project theo đúng đường dẫn

user vào <tên dự án>/Core (đường dẫn đích sẽ là <Thư mục dự án>/Core/user)

trong CMakeLists.txt thêm các dòng sau vào cuối file
	# Add custom User
	add_subdirectory(Core/user)
	target_link_libraries(${CMAKE_PROJECT_NAME}
		service_lib
		app_lib
	)
cũng file CMakeLists.txt thêm vào sau dòng enable_language(C ASM)
	set(CMAKE_C_FLAGS_DEBUG "-Os -g3" CACHE STRING "" FORCE)
	add_compile_options(-Os)

tìm đến đòng sau trong <Thư mục dự án>/cmake/gcc-arm-none-eabi.cmake
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -T \"${CMAKE_SOURCE_DIR}/STM32F411xx_FLASH.ld\"")
sửa thành 
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -T \"${CMAKE_SOURCE_DIR}/Core/user/custom.ld\"")




trong main.c include 2 thư viện sau 
	#include "app/start/appstart.h"
	#include "app/start/command.h"
sau đó sau dòng osKernelInitialize(); thêm mã sau 
	if (appstart_Init(&huart1, &huart2, &hcrc, command_list) != appstart_OK) {
      // Nếu khởi tạo thất bại, có thể dừng chương trình hoặc thực hiện các biện pháp khác
      while (1);
  	}

