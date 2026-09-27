#ifndef DRVUART_H
#define DRVUART_H

#include "main.h"
#include "cmsis_os.h"
#include "stm32f4xx_hal_uart.h"
#include "string.h"

#define MAX_DATA_LENGTH 256

typedef enum {
    DRVUART_OK = 0,
    DRVUART_ERROR,
    DRVUART_BUSY
} drvuart_Status;

typedef struct {
    osMutexId_t mutex_lock_task;
    osSemaphoreId_t sem_data_done;
    uint16_t length;
    uint8_t data[MAX_DATA_LENGTH];
} drvuart_Data;

drvuart_Status drvuart_Init(UART_HandleTypeDef *huart);
drvuart_Status drvuart_DeInit(void);
drvuart_Status drvuart_CheckHealth(void);
drvuart_Status drvuart_Transmit(const uint8_t *data, uint16_t length, uint32_t timeout);
drvuart_Status drvuart_Receive(uint8_t *data, uint16_t *length, uint32_t timeout);


#if !defined(USE_HAL_UART_REGISTER_CALLBACKS) || (USE_HAL_UART_REGISTER_CALLBACKS != 1U)
    #error "[drvUart] Module requires USE_HAL_UART_REGISTER_CALLBACKS to be enabled (1U) in stm32f4xx_hal_conf.h!"
#endif

#endif /* DRVUART_H */