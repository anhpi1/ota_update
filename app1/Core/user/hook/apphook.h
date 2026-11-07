#ifndef APPHOOK_H
#define APPHOOK_H

#include <stdint.h>
#include "hook_guard.h"

/* Địa chỉ bảng router trên RAM do Bootloader nạp lên */
extern uint32_t _s_ram_app_vector[];
#define APP_ROUTER_TABLE _s_ram_app_vector

/* Chỉ số Index tương ứng chính xác 100% với appvectors.s */
enum {
    APP_VEC_MAGIC       = 0,
    APP_VEC_VERSION     = 1,
    APP_VEC_COUNT       = 2,

    /* POSIX / Syscalls */
    APP_VEC_READ        = 4,
    APP_VEC_RESERVED_5  = 5,    /* Reserved (formerly _write, now handled directly in serlog) */
    APP_VEC_SBRK        = 6,
    APP_VEC_CLOSE       = 7,
    APP_VEC_LSEEK       = 8,
    APP_VEC_FSTAT       = 9,
    APP_VEC_ISATTY      = 10,

    /* CubeMX & Hal */
    APP_VEC_SYSCLOCK    = 11,
    APP_VEC_ERROR       = 12,
    APP_VEC_GPIO_INIT   = 13,
    APP_VEC_DMA_INIT    = 14,
    APP_VEC_CRC_INIT    = 15,
    APP_VEC_CRC_MSPINIT = 16,
    APP_VEC_CRC_MSPDEINIT = 17,
    APP_VEC_HAL_MSPINIT = 18,

    /* UART */
    APP_VEC_USART1_INIT = 19,
    APP_VEC_USART2_INIT = 20,
    APP_VEC_UART_MSPINIT = 21,
    APP_VEC_UART_MSPDEINIT = 22,

    /* Timer */
    APP_VEC_TIM_CALLBACK        = 23,
    APP_VEC_INIT_TICK           = 24,
    APP_VEC_SUSPEND_TICK        = 25,
    APP_VEC_RESUME_TICK         = 26,

    /* ========================================================
     * NHÓM BỔ SUNG CÁC HÀM __weak THEO TỪNG MODULE GỐC
     * ======================================================== */
    /* GPIO (trong gpio.c) */
    APP_VEC_GPIO_EXTI_CB        = 27,

    /* UART Callbacks (trong usart.c) */
    APP_VEC_UART_RX_CB          = 28,
    APP_VEC_UART_TX_CB          = 29,
    APP_VEC_UART_ERR_CB         = 30,
    APP_VEC_UART_RX_HALF_CB     = 31,
    APP_VEC_UART_TX_HALF_CB     = 32,
    APP_VEC_UART_RX_EVENT_CB    = 33,

    /* Cortex / SysTick (trong stm32f4xx_hal_msp.c) */
    APP_VEC_SYSTICK_CB          = 34,

    /* TIM Callbacks (trong stm32f4xx_hal_timebase_tim.c) */
    APP_VEC_TIM_OC_CB           = 35,
    APP_VEC_TIM_PWM_CB          = 36,
    APP_VEC_TIM_IC_CB           = 37,
    APP_VEC_TIM_TRIGGER_CB      = 38,

    /* FreeRTOS Hooks (trong freertos.c) */
    APP_VEC_RTOS_IDLE_HOOK      = 39,
    APP_VEC_RTOS_TICK_HOOK      = 40,
    APP_VEC_RTOS_STACK_HOOK     = 41,
    APP_VEC_RTOS_MALLOC_FAIL    = 42,
    APP_VEC_RTOS_DAEMON_HOOK    = 43,

    /* Khởi tạo bổ sung */
    APP_VEC_FREERTOS_INIT       = 44,
    APP_VEC_SYSINIT             = 45,
    APP_VEC_SYSCLOCK_UPDATE     = 46,
    APP_VEC_DEFAULT_TASK        = 47,

    /* UART Abort Callbacks (trong usart.c) */
    APP_VEC_UART_ABORT_CB       = 48,
    APP_VEC_UART_ABORT_TX_CB    = 49,
    APP_VEC_UART_ABORT_RX_CB    = 50,

    /* Timer Half-complete Callbacks */
    APP_VEC_TIM_PERIOD_HALF_CB  = 51,
    APP_VEC_TIM_PWM_HALF_CB     = 52,

    /* Nguồn & Xung nhịp Callbacks */
    APP_VEC_PWR_PVD_CB          = 53,
    APP_VEC_RCC_CSS_CB          = 54,

    /* Flash Callbacks */
    APP_VEC_FLASH_END_CB        = 55,
    APP_VEC_FLASH_ERR_CB        = 56,

    /* Biến & Bảng hằng số Clock hệ thống (trong system_stm32f4xx.c) */
    APP_VEC_SYSCLOCK_VAR        = 57,
    APP_VEC_AHB_PRESC_TABLE     = 58,
    APP_VEC_APB_PRESC_TABLE     = 59
};

#endif /* APPHOOK_H */