# ==============================================================================
# HOOK LIBRARY (Đường hầm Trampoline chuyển tiếp từ Service sang App qua RAM)
# ==============================================================================
add_library(hook_lib OBJECT)

# 1. Danh sách toàn bộ các file Hook chứa hàm __wrap_*
target_sources(hook_lib PRIVATE
    Core/Src/crc.c
    Core/Src/dma.c
    Core/Src/freertos.c
    Core/Src/gpio.c
    Core/Src/main.c
    Core/Src/stm32f4xx_hal_msp.c
    Core/Src/stm32f4xx_hal_timebase_tim.c
    Core/Src/stm32f4xx_it.c
    Core/Src/syscalls.c
    Core/Src/sysmem.c
    Core/Src/system_stm32f4xx.c
    Core/Src/usart.c
)

# Chỉ cần include thư mục hiện tại để tìm các header cục bộ (nếu có)
target_include_directories(hook_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

# Thừa hưởng toàn bộ HAL, FreeRTOS, CMSIS từ stm32cubemx
target_link_libraries(hook_lib PUBLIC
    stm32cubemx
)

# ==============================================================================
# 4. DANH SÁCH CÁC CỜ BẺ LÁI LINKER (--wrap=<symbol>)
# Ép toàn bộ lời gọi hàm trong firmware phải chui qua các hàm __wrap_* tương ứng
# ==============================================================================
target_link_options(${CMAKE_PROJECT_NAME} PRIVATE
    # C/POSIX Syscalls (chặn Newlib gọi tĩnh)
    "-Wl,--wrap=_read"
    "-Wl,--wrap=_sbrk"
    "-Wl,--wrap=_close"
    "-Wl,--wrap=_fstat"
    "-Wl,--wrap=_isatty"
    "-Wl,--wrap=_lseek"
    "-Wl,--wrap=_open"
    "-Wl,--wrap=_wait"
    "-Wl,--wrap=_unlink"
    "-Wl,--wrap=_times"
    "-Wl,--wrap=_stat"
    "-Wl,--wrap=_link"
    "-Wl,--wrap=_fork"
    "-Wl,--wrap=_execve"
    "-Wl,--wrap=_exit"
    "-Wl,--wrap=_kill"
    "-Wl,--wrap=_getpid"

    # HAL Callbacks & MspInit
    "-Wl,--wrap=HAL_CRC_MspInit"
    "-Wl,--wrap=HAL_CRC_MspDeInit"
    "-Wl,--wrap=HAL_UART_MspInit"
    "-Wl,--wrap=HAL_UART_MspDeInit"
    "-Wl,--wrap=HAL_TIM_PeriodElapsedCallback"
    "-Wl,--wrap=HAL_MspInit"
    "-Wl,--wrap=HAL_InitTick"
    "-Wl,--wrap=HAL_SuspendTick"
    "-Wl,--wrap=HAL_ResumeTick"
    "-Wl,--wrap=HAL_GPIO_EXTI_Callback"
    "-Wl,--wrap=HAL_UART_RxCpltCallback"
    "-Wl,--wrap=HAL_UART_TxCpltCallback"
    "-Wl,--wrap=HAL_UART_ErrorCallback"
    "-Wl,--wrap=HAL_UART_RxHalfCpltCallback"
    "-Wl,--wrap=HAL_UART_TxHalfCpltCallback"
    "-Wl,--wrap=HAL_UARTEx_RxEventCallback"
    "-Wl,--wrap=HAL_UART_AbortCpltCallback"
    "-Wl,--wrap=HAL_UART_AbortTransmitCpltCallback"
    "-Wl,--wrap=HAL_UART_AbortReceiveCpltCallback"
    "-Wl,--wrap=HAL_SYSTICK_Callback"
    "-Wl,--wrap=HAL_TIM_OC_DelayElapsedCallback"
    "-Wl,--wrap=HAL_TIM_PWM_PulseFinishedCallback"
    "-Wl,--wrap=HAL_TIM_IC_CaptureCallback"
    "-Wl,--wrap=HAL_TIM_TriggerCallback"
    "-Wl,--wrap=HAL_TIM_PeriodElapsedHalfCpltCallback"
    "-Wl,--wrap=HAL_TIM_PWM_PulseFinishedHalfCpltCallback"

    # Nguồn, Xung nhịp & Flash Callbacks
    "-Wl,--wrap=HAL_PWR_PVDCallback"
    "-Wl,--wrap=HAL_RCC_CSSCallback"
    "-Wl,--wrap=HAL_FLASH_EndOfOperationCallback"
    "-Wl,--wrap=HAL_FLASH_OperationErrorCallback"

    # FreeRTOS Application Hooks
    "-Wl,--wrap=vApplicationIdleHook"
    "-Wl,--wrap=vApplicationTickHook"
    "-Wl,--wrap=vApplicationStackOverflowHook"
    "-Wl,--wrap=vApplicationMallocFailedHook"
    "-Wl,--wrap=vApplicationDaemonTaskStartupHook"

    # Khởi tạo CubeMX & System
    "-Wl,--wrap=SystemClock_Config"
    "-Wl,--wrap=Error_Handler"
    "-Wl,--wrap=SystemInit"
    "-Wl,--wrap=SystemCoreClockUpdate"
    "-Wl,--wrap=SystemCoreClock"
    "-Wl,--wrap=AHBPrescTable"
    "-Wl,--wrap=APBPrescTable"
    "-Wl,--wrap=MX_GPIO_Init"
    "-Wl,--wrap=MX_DMA_Init"
    "-Wl,--wrap=MX_CRC_Init"
    "-Wl,--wrap=MX_USART1_UART_Init"
    "-Wl,--wrap=MX_USART2_UART_Init"
    "-Wl,--wrap=MX_FREERTOS_Init"
    "-Wl,--wrap=StartDefaultTask"
)