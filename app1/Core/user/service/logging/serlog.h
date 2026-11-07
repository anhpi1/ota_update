#ifndef SERLOG_H
#define SERLOG_H

// driver log kết nối lệnh printf với uart để in ra log, có thể sử dụng printf để in ra log
// về bản chất thư viện này sử dụng printf để in log, nên nếu sử dụng printf nên khới tạo thư viện này
// sử dụng RTOS 
// không sử dụng hàm này trong ngắt


#include "stm32f4xx_hal.h"
#include "stdio.h"
#include "cmsis_os.h"
#include "stm32f4xx_hal_uart.h"
#include "string.h"
#include "usersetting.h"
#include "stdint.h"
#include "driver/uart/drvUart.h"

#define SERLOG_MAX_LENGTH SET_LOG_MAX_LENGTH 
#define SERLOG_MAX_QUEUE_LENGTH SET_LOG_MAX_QUEUE_LENGTH 
#define SERLOG_TIMEOUT SET_LOG_TIMEOUT // Timeout in milliseconds for queue operations

#define SERLOG_STACK_CHECK SET_LOG_STACK_CHECK // Bật/tắt tính năng kiểm tra dung lượng stack còn trống của task log


typedef enum {
    serlog_OK=0,
    serlog_ERROR,
    serlog_QUEUE_FULL,
    serlog_UART_BUSY,
    serlog_QUEUE_EMPTY,
    serlog_TIMEOUT
} serlog_Status;

typedef struct {
    char buff[SERLOG_MAX_LENGTH];
    uint8_t volatile length;
} serlog_Element;



serlog_Status serlog_Init(drvuart_Handle *handle);
serlog_Status serlog_DeInit(void);
serlog_Status serlog_CheckHealth(void);
const char* get_filename(const char *path);



#if SERLOG_STACK_CHECK
    uint32_t serlog_GetTaskStackSpace(void);
#endif




#if SERLOG_DEBUG
    #define SERLOG_KEY_I "INFO"
    #define SERLOG_KEY_W "WARNING"
    #define SERLOG_KEY_E "ERROR"

    #define SERLOG(key, format, ...) \
        do { \
            printf("[" SERLOG_KEY_##key "] file:\"%s\" line:\"%d\" msg: " format "\r\n", \
                   get_filename(__FILE__), __LINE__, ##__VA_ARGS__); \
        } while(0)
#else
    #define SERLOG(key, format, ...) do { (void)0; } while(0)
#endif





#endif /* SERLOG_H */