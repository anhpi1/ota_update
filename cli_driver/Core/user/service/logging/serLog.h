#ifndef SERLOG_H
#define SERLOG_H

// driver log kết nối lệnh printf với uart để in ra log, có thể sử dụng printf để in ra log
// về bản chất thư viện này sử dụng printf để in log, nên nếu sử dụng printf nên khới tạo thư viện này
// sử dụng RTOS 
// không sử dụng hàm này trong ngắt

#include "main.h"
#include "stdio.h"
#include "cmsis_os.h"
#include "stm32f4xx_hal_uart.h"
#include "string.h"
#include "setting.h"
#include "stdint.h"
#include "driver/uart/drvUart.h"

#define LOG_MAX_LENGTH SET_LOG_MAX_LENGTH 
#define LOG_MAX_QUEUE_LENGTH SET_LOG_MAX_QUEUE_LENGTH 
#define LOG_TIMEOUT SET_LOG_TIMEOUT // Timeout in milliseconds for queue operations

#define LOG_STACK_CHECK SET_LOG_STACK_CHECK // Bật/tắt tính năng kiểm tra dung lượng stack còn trống của task log


typedef enum {
    logS_ok=0,
    logS_error,
    logS_queue_full,
    logS_uart_busy,
    logS_queue_empty,
    logS_timeout
} log_Status;

typedef struct {
    char buff[LOG_MAX_LENGTH];
    uint8_t volatile length;
} log_Element;



log_Status log_Init(drvuart_Handle *handle);
log_Status log_DeInit(void);
log_Status log_CheckHealth(void);
const char* get_filename(const char *path);



#if LOG_STACK_CHECK
    uint32_t log_GetTaskStackSpace(void);
#endif



#if LOG_DEBUG
    #define LOG_KEY_I "INFO"
    #define LOG_KEY_W "WARNING"
    #define LOG_KEY_E "ERROR"

    #define LOG(key, format, ...) \
        do { \
            printf("[" LOG_KEY_##key "] file:\"%s\" line:\"%d\" msg: " format "\r\n", \
                   get_filename(__FILE__), __LINE__, ##__VA_ARGS__); \
        } while(0)
#else
    #define LOG(key, format, ...) do { (void)0; } while(0)
#endif





#endif /* SERLOG_H */