.syntax unified
.cpu cortex-m4
.thumb

/* Bảng Vector Hàm của App đặt trong section .app_vector */
.section .app_vector, "a"
.global g_appVectors

g_appVectors:
    /* ---------------------------------------------------------
     * 1. HEADER NHẬN DIỆN & PHIÊN BẢN (Offset 0x00 - 0x0F)
     * --------------------------------------------------------- */
    .word 0x41505031          /* Word 0: Magic "APP1" (0x41505031) xác thực App hợp lệ */
    .word 1                   /* Word 1: Version cấu trúc bảng vector (Version = 1) */
    .word 128                 /* Word 2: Tổng số lượng entry trong bảng này (128 words = 512 bytes) */
    .word 0                   /* Word 3: Dự phòng (Reserved) */

    /* ---------------------------------------------------------
     * 2. NHÓM SYSCALLS & POSIX I/O (Offset 0x10 - 0x2B)
     * Newlib trên Service gọi xuống App khi printf/malloc
     * --------------------------------------------------------- */
    .word __real__read               /* Word 4:  Hàm đọc dữ liệu I/O (syscalls.c) */
    .word 0                          /* Word 5:  Reserved (formerly _write, handled directly in serlog) */
    .word __real__sbrk               /* Word 6:  Hàm cấp phát Heap (sysmem.c) */
    .word __real__close              /* Word 7:  Hàm đóng file (syscalls.c) */
    .word __real__lseek              /* Word 8:  Hàm định vị con trỏ file (syscalls.c) */
    .word __real__fstat              /* Word 9:  Hàm trạng thái file (syscalls.c) */
    .word __real__isatty             /* Word 10: Hàm kiểm tra thiết bị terminal (syscalls.c) */

    /* ---------------------------------------------------------
     * 3. NHÓM NGOẠI VI & KHỞI TẠO CUBEMX (Offset 0x2C - 0x4B)
     * --------------------------------------------------------- */
    .word __real_SystemClock_Config  /* Word 11: Cấu hình xung nhịp hệ thống (main.c) */
    .word __real_Error_Handler       /* Word 12: Bẫy lỗi phần cứng (main.c) */
    .word __real_MX_GPIO_Init        /* Word 13: Khởi tạo GPIO (gpio.c) */
    .word __real_MX_DMA_Init         /* Word 14: Khởi tạo DMA (dma.c) */
    .word __real_MX_CRC_Init         /* Word 15: Khởi tạo ngoại vi CRC (crc.c) */
    .word __real_HAL_CRC_MspInit     /* Word 16: Bật clock phần cứng CRC (crc.c) */
    .word __real_HAL_CRC_MspDeInit   /* Word 17: Tắt clock phần cứng CRC (crc.c) */
    .word __real_HAL_MspInit         /* Word 18: Khởi tạo Global MSP (stm32f4xx_hal_msp.c) */

    /* ---------------------------------------------------------
     * 4. NHÓM UART CỦA APP (Offset 0x4C - 0x5B)
     * --------------------------------------------------------- */
    .word __real_MX_USART1_UART_Init /* Word 19: Cấu hình USART1 (usart.c) */
    .word __real_MX_USART2_UART_Init /* Word 20: Cấu hình USART2 (usart.c) */
    .word __real_HAL_UART_MspInit    /* Word 21: Cấu hình GPIO Alternate & Clock UART (usart.c) */
    .word __real_HAL_UART_MspDeInit  /* Word 22: Giải phóng chân UART (usart.c) */

    /* ---------------------------------------------------------
     * 5. NHÓM TIMER & TIMEBASE TICK (Offset 0x5C - 0x6B)
     * --------------------------------------------------------- */
    .word __real_HAL_TIM_PeriodElapsedCallback /* Word 23: Callback ngắt TIM1 tăng uwTick (main.c) */
    .word __real_HAL_InitTick                  /* Word 24: Khởi tạo Timer timebase (hal_timebase_tim.c) */
    .word __real_HAL_SuspendTick               /* Word 25: Tạm dừng ngắt Timer (hal_timebase_tim.c) */
    .word __real_HAL_ResumeTick                /* Word 26: Tiếp tục ngắt Timer (hal_timebase_tim.c) */

    /* ---------------------------------------------------------
     * 6. NHÓM CALLBACKS __weak CỦA HAL & FREERTOS (Offset 0x6C - 0xAF)
     * --------------------------------------------------------- */
    .weak __real_HAL_GPIO_EXTI_Callback
    .word __real_HAL_GPIO_EXTI_Callback        /* Word 27: Callback ngắt ngoài EXTI (gpio.c) */

    .weak __real_HAL_UART_RxCpltCallback
    .word __real_HAL_UART_RxCpltCallback       /* Word 28: Nhận xong UART DMA/IT (usart.c) */

    .weak __real_HAL_UART_TxCpltCallback
    .word __real_HAL_UART_TxCpltCallback       /* Word 29: Truyền xong UART DMA/IT (usart.c) */

    .weak __real_HAL_UART_ErrorCallback
    .word __real_HAL_UART_ErrorCallback        /* Word 30: Báo lỗi UART (usart.c) */

    .weak __real_HAL_UART_RxHalfCpltCallback
    .word __real_HAL_UART_RxHalfCpltCallback   /* Word 31: Nhận nửa buffer DMA UART (usart.c) */

    .weak __real_HAL_UART_TxHalfCpltCallback
    .word __real_HAL_UART_TxHalfCpltCallback   /* Word 32: Truyền nửa buffer DMA UART (usart.c) */

    .weak __real_HAL_UARTEx_RxEventCallback
    .word __real_HAL_UARTEx_RxEventCallback    /* Word 33: Nhận IDLE Event UART (usart.c) */

    .weak __real_HAL_SYSTICK_Callback
    .word __real_HAL_SYSTICK_Callback          /* Word 34: Callback ngắt SysTick (stm32f4xx_hal_msp.c) */

    .weak __real_HAL_TIM_OC_DelayElapsedCallback
    .word __real_HAL_TIM_OC_DelayElapsedCallback /* Word 35: Timer Output Compare (stm32f4xx_hal_timebase_tim.c) */

    .weak __real_HAL_TIM_PWM_PulseFinishedCallback
    .word __real_HAL_TIM_PWM_PulseFinishedCallback /* Word 36: Timer PWM pulse finish (stm32f4xx_hal_timebase_tim.c) */

    .weak __real_HAL_TIM_IC_CaptureCallback
    .word __real_HAL_TIM_IC_CaptureCallback    /* Word 37: Timer Input Capture (stm32f4xx_hal_timebase_tim.c) */

    .weak __real_HAL_TIM_TriggerCallback
    .word __real_HAL_TIM_TriggerCallback       /* Word 38: Timer Trigger Event (stm32f4xx_hal_timebase_tim.c) */

    .weak __real_vApplicationIdleHook
    .word __real_vApplicationIdleHook          /* Word 39: FreeRTOS Idle Task Hook (freertos.c) */

    .weak __real_vApplicationTickHook
    .word __real_vApplicationTickHook          /* Word 40: FreeRTOS Tick Hook (freertos.c) */

    .weak __real_vApplicationStackOverflowHook
    .word __real_vApplicationStackOverflowHook /* Word 41: Bẫy tràn Stack FreeRTOS (freertos.c) */

    .weak __real_vApplicationMallocFailedHook
    .word __real_vApplicationMallocFailedHook  /* Word 42: Bẫy hết Heap FreeRTOS (freertos.c) */

    .weak __real_vApplicationDaemonTaskStartupHook
    .word __real_vApplicationDaemonTaskStartupHook /* Word 43: FreeRTOS Daemon Task Hook (freertos.c) */

    .weak __real_MX_FREERTOS_Init
    .word __real_MX_FREERTOS_Init              /* Word 44: Khởi tạo FreeRTOS App (freertos.c) */

    .weak __real_SystemInit
    .word __real_SystemInit                    /* Word 45: Khởi tạo chip cơ sở (system_stm32f4xx.c) */

    .weak __real_SystemCoreClockUpdate
    .word __real_SystemCoreClockUpdate         /* Word 46: Cập nhật biến tần số HCLK (system_stm32f4xx.c) */

    .weak __real_StartDefaultTask
    .word __real_StartDefaultTask              /* Word 47: Luồng mặc định FreeRTOS (freertos.c) */

    .weak __real_HAL_UART_AbortCpltCallback
    .word __real_HAL_UART_AbortCpltCallback    /* Word 48: Callback hủy UART toàn phần (usart.c) */

    .weak __real_HAL_UART_AbortTransmitCpltCallback
    .word __real_HAL_UART_AbortTransmitCpltCallback /* Word 49: Callback hủy truyền UART (usart.c) */

    .weak __real_HAL_UART_AbortReceiveCpltCallback
    .word __real_HAL_UART_AbortReceiveCpltCallback /* Word 50: Callback hủy nhận UART (usart.c) */

    .weak __real_HAL_TIM_PeriodElapsedHalfCpltCallback
    .word __real_HAL_TIM_PeriodElapsedHalfCpltCallback     /* Word 51: Callback nửa chu kỳ Timer */

    .weak __real_HAL_TIM_PWM_PulseFinishedHalfCpltCallback
    .word __real_HAL_TIM_PWM_PulseFinishedHalfCpltCallback /* Word 52: Callback nửa chu kỳ PWM */

    .weak __real_HAL_PWR_PVDCallback
    .word __real_HAL_PWR_PVDCallback                       /* Word 53: Callback cảnh báo sụt áp PVD */

    .weak __real_HAL_RCC_CSSCallback
    .word __real_HAL_RCC_CSSCallback                       /* Word 54: Callback mất xung Clock CSS */

    .weak __real_HAL_FLASH_EndOfOperationCallback
    .word __real_HAL_FLASH_EndOfOperationCallback          /* Word 55: Callback Flash kết thúc ghi/xóa */

    .weak __real_HAL_FLASH_OperationErrorCallback
    .word __real_HAL_FLASH_OperationErrorCallback          /* Word 56: Callback Flash báo lỗi */

    /* ---------------------------------------------------------
     * 7. NHÓM BIẾN HỆ THỐNG & DỰ PHÒNG MỞ RỘNG (Word 57 - Word 127)
     * --------------------------------------------------------- */
    .word __real_SystemCoreClock              /* Word 57: Địa chỉ biến SystemCoreClock của App */
    .word __real_AHBPrescTable                /* Word 58: Địa chỉ bảng AHBPrescTable của App */
    .word __real_APBPrescTable                /* Word 59: Địa chỉ bảng APBPrescTable của App */
    .rept 68
    .word 0                                   /* Word 60..127: Dự phòng mở rộng (68 words) */
    .endr

    .size g_appVectors, .-g_appVectors