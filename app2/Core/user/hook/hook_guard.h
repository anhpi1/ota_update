#ifndef HOOK_GUARD_H
#define HOOK_GUARD_H

#include "main.h"
#include "FreeRTOSConfig.h"

/* ==============================================================================
 * PHẦN 1: CÁC FILE HAL DRIVER ĐANG NẰM TRÊN SERVICE FLASH (SECTOR 7)
 * (Các module mới như SPI, I2C, ADC, CAN... không nằm trên Service nên App tự do đổi)
 * ============================================================================== */

/* 1.1 Kiểm tra các module HAL cốt lõi của Service */
#ifndef HAL_MODULE_ENABLED
  #error "[SERVICE DESYNC] HAL_MODULE_ENABLED bị tắt! BẠN PHẢI CẬP NHẬT LẠI SERVICE FLASH (SECTOR 7)!"
#endif
#ifndef HAL_CRC_MODULE_ENABLED
  #error "[SERVICE DESYNC] HAL_CRC_MODULE_ENABLED bị tắt! Service cần CRC phần cứng. BẠN PHẢI CẬP NHẬT LẠI SERVICE FLASH (SECTOR 7)!"
#endif
#ifndef HAL_UART_MODULE_ENABLED
  #error "[SERVICE DESYNC] HAL_UART_MODULE_ENABLED bị tắt! Service CLI/OTA cần UART. BẠN PHẢI CẬP NHẬT LẠI SERVICE FLASH (SECTOR 7)!"
#endif
#ifndef HAL_DMA_MODULE_ENABLED
  #error "[SERVICE DESYNC] HAL_DMA_MODULE_ENABLED bị tắt! Service cần DMA UART. BẠN PHẢI CẬP NHẬT LẠI SERVICE FLASH (SECTOR 7)!"
#endif
#ifndef HAL_GPIO_MODULE_ENABLED
  #error "[SERVICE DESYNC] HAL_GPIO_MODULE_ENABLED bị tắt! BẠN PHẢI CẬP NHẬT LẠI SERVICE FLASH (SECTOR 7)!"
#endif
#ifndef HAL_EXTI_MODULE_ENABLED
  #error "[SERVICE DESYNC] HAL_EXTI_MODULE_ENABLED bị tắt! BẠN PHẢI CẬP NHẬT LẠI SERVICE FLASH (SECTOR 7)!"
#endif
#ifndef HAL_TIM_MODULE_ENABLED
  #error "[SERVICE DESYNC] HAL_TIM_MODULE_ENABLED bị tắt! Timebase HAL cần TIM. BẠN PHẢI CẬP NHẬT LẠI SERVICE FLASH (SECTOR 7)!"
#endif
#ifndef HAL_FLASH_MODULE_ENABLED
  #error "[SERVICE DESYNC] HAL_FLASH_MODULE_ENABLED bị tắt! FOTA cần Flash. BẠN PHẢI CẬP NHẬT LẠI SERVICE FLASH (SECTOR 7)!"
#endif
#ifndef HAL_RCC_MODULE_ENABLED
  #error "[SERVICE DESYNC] HAL_RCC_MODULE_ENABLED bị tắt! BẠN PHẢI CẬP NHẬT LẠI SERVICE FLASH (SECTOR 7)!"
#endif
#ifndef HAL_PWR_MODULE_ENABLED
  #error "[SERVICE DESYNC] HAL_PWR_MODULE_ENABLED bị tắt! BẠN PHẢI CẬP NHẬT LẠI SERVICE FLASH (SECTOR 7)!"
#endif
#ifndef HAL_CORTEX_MODULE_ENABLED
  #error "[SERVICE DESYNC] HAL_CORTEX_MODULE_ENABLED bị tắt! BẠN PHẢI CẬP NHẬT LẠI SERVICE FLASH (SECTOR 7)!"
#endif

/* 1.2 Kiểm tra Layout struct HAL & Xung nhịp gốc (Tránh lệch kích thước struct Handle / tần số) */
#if !defined(USE_HAL_UART_REGISTER_CALLBACKS) || (USE_HAL_UART_REGISTER_CALLBACKS != 1U)
  #error "[SERVICE DESYNC] USE_HAL_UART_REGISTER_CALLBACKS != 1U! Cấu trúc UART_HandleTypeDef bị lệch. BẠN PHẢI CẬP NHẬT LẠI SERVICE FLASH (SECTOR 7)!"
#endif

#if !defined(USE_HAL_TIM_REGISTER_CALLBACKS) || (USE_HAL_TIM_REGISTER_CALLBACKS != 0U)
  #error "[SERVICE DESYNC] USE_HAL_TIM_REGISTER_CALLBACKS != 0U! Cấu trúc TIM_HandleTypeDef bị lệch. BẠN PHẢI CẬP NHẬT LẠI SERVICE FLASH (SECTOR 7)!"
#endif

#if !defined(USE_RTOS) || (USE_RTOS != 0U)
  #error "[SERVICE DESYNC] USE_RTOS trong stm32f4xx_hal_conf.h != 0U! BẠN PHẢI CẬP NHẬT LẠI SERVICE FLASH (SECTOR 7)!"
#endif

#if !defined(HSE_VALUE) || (HSE_VALUE != 25000000U)
  #error "[SERVICE DESYNC] HSE_VALUE bị đổi khỏi 25000000U! Hàm HAL_RCC trên Sector 7 sẽ tính sai tần số. BẠN PHẢI CẬP NHẬT LẠI SERVICE FLASH (SECTOR 7)!"
#endif

#ifndef HSI_VALUE
  #error "[SERVICE DESYNC] HSI_VALUE bị thiếu! BẠN PHẢI CẬP NHẬT LẠI SERVICE FLASH (SECTOR 7)!"
#endif
/* HSI_VALUE có ép kiểu ((uint32_t)...) nên không dùng được trong #if -> dùng _Static_assert */
_Static_assert(HSI_VALUE == 16000000U, "[SERVICE DESYNC] HSI_VALUE bi doi khoi 16000000U! BAN PHAI CAP NHAT LAI SERVICE FLASH (SECTOR 7)!");

/* 1.3 Cảnh báo khi bật thêm ngoại vi mới (Chạy ở FLASH_APP / RAM_APP) */
#if defined(HAL_SPI_MODULE_ENABLED) || defined(HAL_I2C_MODULE_ENABLED) || \
    defined(HAL_ADC_MODULE_ENABLED) || defined(HAL_IWDG_MODULE_ENABLED) || \
    defined(HAL_WWDG_MODULE_ENABLED) || defined(HAL_RTC_MODULE_ENABLED)
  #warning "[APP PERIPHERAL NOTICE] Ngoại vi mới (SPI/I2C/ADC/WDG/RTC) được bật sẽ liên kết tự động vào FLASH_APP & RAM_APP."
#endif

/* ==============================================================================
 * PHẦN 2: CẤU HÌNH FREERTOS KERNEL ĐANG NẰM TRÊN SERVICE FLASH (SECTOR 7)
 * (Lệch bất kỳ cấu hình nào dưới đây sẽ làm vỡ cấu trúc TCB / Queue / Ticks / .service_bss)
 * ============================================================================== */

/* 2.1 Cấu hình kích thước và trường của TCB (Task Control Block) & .service_bss */
#if !defined(configUSE_PREEMPTION) || (configUSE_PREEMPTION != 1)
  #error "[SERVICE DESYNC] configUSE_PREEMPTION != 1! BẠN PHẢI CẬP NHẬT LẠI SERVICE FLASH (SECTOR 7)!"
#endif

#if !defined(configMAX_PRIORITIES) || (configMAX_PRIORITIES != 56)
  #error "[SERVICE DESYNC] configMAX_PRIORITIES bị đổi khỏi 56! Mảng Ready Tasks bị lệch kích thước. BẠN PHẢI CẬP NHẬT LẠI SERVICE FLASH (SECTOR 7)!"
#endif

#if !defined(configMAX_TASK_NAME_LEN) || (configMAX_TASK_NAME_LEN != 16)
  #error "[SERVICE DESYNC] configMAX_TASK_NAME_LEN bị đổi khỏi 16! Kích thước struct TCB bị lệch. BẠN PHẢI CẬP NHẬT LẠI SERVICE FLASH (SECTOR 7)!"
#endif

#if !defined(configUSE_TRACE_FACILITY) || (configUSE_TRACE_FACILITY != 1)
  #error "[SERVICE DESYNC] configUSE_TRACE_FACILITY bị đổi khỏi 1! Cấu trúc TCB bị lệch. BẠN PHẢI CẬP NHẬT LẠI SERVICE FLASH (SECTOR 7)!"
#endif

#if !defined(configUSE_16_BIT_TICKS) || (configUSE_16_BIT_TICKS != 0)
  #error "[SERVICE DESYNC] configUSE_16_BIT_TICKS != 0! Kiểu TickType_t bị đổi kích thước. BẠN PHẢI CẬP NHẬT LẠI SERVICE FLASH (SECTOR 7)!"
#endif

#if !defined(configUSE_MUTEXES) || (configUSE_MUTEXES != 1)
  #error "[SERVICE DESYNC] configUSE_MUTEXES != 1! Mutex bị tắt. BẠN PHẢI CẬP NHẬT LẠI SERVICE FLASH (SECTOR 7)!"
#endif

#if !defined(configUSE_RECURSIVE_MUTEXES) || (configUSE_RECURSIVE_MUTEXES != 1)
  #error "[SERVICE DESYNC] configUSE_RECURSIVE_MUTEXES != 1! BẠN PHẢI CẬP NHẬT LẠI SERVICE FLASH (SECTOR 7)!"
#endif

#if !defined(configUSE_COUNTING_SEMAPHORES) || (configUSE_COUNTING_SEMAPHORES != 1)
  #error "[SERVICE DESYNC] configUSE_COUNTING_SEMAPHORES != 1! BẠN PHẢI CẬP NHẬT LẠI SERVICE FLASH (SECTOR 7)!"
#endif

#if !defined(configSUPPORT_STATIC_ALLOCATION) || (configSUPPORT_STATIC_ALLOCATION != 1)
  #error "[SERVICE DESYNC] configSUPPORT_STATIC_ALLOCATION != 1! Cấu trúc TCB và bộ đệm Idle/Timer Task bị lệch. BẠN PHẢI CẬP NHẬT LẠI SERVICE FLASH (SECTOR 7)!"
#endif

#if !defined(configSUPPORT_DYNAMIC_ALLOCATION) || (configSUPPORT_DYNAMIC_ALLOCATION != 1)
  #error "[SERVICE DESYNC] configSUPPORT_DYNAMIC_ALLOCATION != 1! BẠN PHẢI CẬP NHẬT LẠI SERVICE FLASH (SECTOR 7)!"
#endif

#if !defined(configQUEUE_REGISTRY_SIZE) || (configQUEUE_REGISTRY_SIZE != 8)
  #error "[SERVICE DESYNC] configQUEUE_REGISTRY_SIZE bị đổi khỏi 8! Mảng xQueueRegistry trên RAM_SERVICE bị lệch. BẠN PHẢI CẬP NHẬT LẠI SERVICE FLASH (SECTOR 7)!"
#endif

/* Hai macro dưới có ép kiểu ((uint16_t)128), ((size_t)15360) nên không dùng được trong #if -> dùng _Static_assert */
#ifndef configMINIMAL_STACK_SIZE
  #error "[SERVICE DESYNC] configMINIMAL_STACK_SIZE bị thiếu! BẠN PHẢI CẬP NHẬT LẠI SERVICE FLASH (SECTOR 7)!"
#endif
_Static_assert(configMINIMAL_STACK_SIZE == 128, "[SERVICE DESYNC] configMINIMAL_STACK_SIZE bi doi khoi 128! Mang Idle_Stack tren RAM_SERVICE bi lech. BAN PHAI CAP NHAT LAI SERVICE FLASH (SECTOR 7)!");

#ifndef configTOTAL_HEAP_SIZE
  #error "[SERVICE DESYNC] configTOTAL_HEAP_SIZE bị thiếu! BẠN PHẢI CẬP NHẬT LẠI SERVICE FLASH (SECTOR 7)!"
#endif
_Static_assert(configTOTAL_HEAP_SIZE == 15360, "[SERVICE DESYNC] configTOTAL_HEAP_SIZE bi doi khoi 15360! Mang ucHeap tren RAM_SERVICE bi lech. BAN PHAI CAP NHAT LAI SERVICE FLASH (SECTOR 7)!");

/* 2.2 Cấu hình Nhịp độ & Timer & Ngắt */
#ifndef configTICK_RATE_HZ
  #error "[SERVICE DESYNC] configTICK_RATE_HZ bị thiếu! BẠN PHẢI CẬP NHẬT LẠI SERVICE FLASH (SECTOR 7)!"
#endif

#if !defined(configUSE_TIMERS) || (configUSE_TIMERS != 1)
  #error "[SERVICE DESYNC] configUSE_TIMERS != 1! BẠN PHẢI CẬP NHẬT LẠI SERVICE FLASH (SECTOR 7)!"
#endif

#if !defined(configTIMER_QUEUE_LENGTH) || (configTIMER_QUEUE_LENGTH != 10)
  #error "[SERVICE DESYNC] configTIMER_QUEUE_LENGTH bị đổi khỏi 10! Mảng ucStaticTimerQueueStorage trên RAM_SERVICE bị lệch. BẠN PHẢI CẬP NHẬT LẠI SERVICE FLASH (SECTOR 7)!"
#endif

#if !defined(configTIMER_TASK_STACK_DEPTH) || (configTIMER_TASK_STACK_DEPTH != 256)
  #error "[SERVICE DESYNC] configTIMER_TASK_STACK_DEPTH bị đổi khỏi 256! Mảng Timer_Stack trên RAM_SERVICE bị lệch. BẠN PHẢI CẬP NHẬT LẠI SERVICE FLASH (SECTOR 7)!"
#endif

#if !defined(configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY) || (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY != 5)
  #error "[SERVICE DESYNC] configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY bị đổi khỏi 5! Mức ưu tiên ngắt FreeRTOS bị lệch. BẠN PHẢI CẬP NHẬT LẠI SERVICE FLASH (SECTOR 7)!"
#endif

#endif /* HOOK_GUARD_H */
