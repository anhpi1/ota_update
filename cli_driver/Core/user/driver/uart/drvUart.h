#ifndef DRVUART_H
#define DRVUART_H

#include "main.h"
#include "cmsis_os.h"
#include "stm32f4xx_hal_uart.h"
#include "string.h"
#include "setting.h"

// file driver này đảm nhận vai trò:
// an toàn đa luồng khóa tài nguyên khi đang có task sử dụng
// truyền nhận dữ liệu an toàn khi sử dụng DMA
// giảm tải cpu bằng cách dùng semaphore để chờ dữ liệu truyền nhận xong
// cần phải bật tính năng #define  USE_HAL_UART_REGISTER_CALLBACKS        1U 

#define DRVUART_MAX_DATA_LENGTH SET_DRVUART_MAX_DATA_LENGTH
#define DRVUART_MAX_INSTANCES SET_DRVUART_MAX_INSTANCES

typedef enum {
    DRVUART_OK = 0,
    DRVUART_ERROR,
    DRVUART_BUSY
} drvuart_Status;

typedef struct {
    osMutexId_t mutex_lock_task;
    osSemaphoreId_t sem_data_done;
    uint16_t length;
    uint8_t data[DRVUART_MAX_DATA_LENGTH];
} drvuart_Data;

typedef struct {
    UART_HandleTypeDef *huart;
    drvuart_Data        tx;
    drvuart_Data        rx;
} drvuart_Handle;

drvuart_Status drvuart_Init(drvuart_Handle *handle, UART_HandleTypeDef *huart);
drvuart_Status drvuart_DeInit(drvuart_Handle *handle);
drvuart_Status drvuart_CheckHealth(drvuart_Handle *handle);
drvuart_Status drvuart_Transmit(drvuart_Handle *handle, const uint8_t *data, uint16_t length, uint32_t timeout);
drvuart_Status drvuart_Receive(drvuart_Handle *handle, uint8_t *data, uint16_t *length, uint32_t timeout);


#if !defined(USE_HAL_UART_REGISTER_CALLBACKS) || (USE_HAL_UART_REGISTER_CALLBACKS != 1U)
    #error "[drvUart] Module requires USE_HAL_UART_REGISTER_CALLBACKS to be enabled (1U) in stm32f4xx_hal_conf.h!"
#endif

#endif /* DRVUART_H */