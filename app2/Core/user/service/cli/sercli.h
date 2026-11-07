#ifndef SERCLI_H
#define SERCLI_H

// cli được thiết kế theo dạng chỉ xử lý 1 lệnh rồi mới nhận lệnh tiếp theo không có hàng đợi thực thi lệnh
// vì nó không cần thiết, tốc độ nhập của con người khá chậm và tốc độ thực thi lệnh cũng khá nhanh
// cli phụ thuộc vào log, vì vậy cần khởi tạo log trước khi khởi tạo cli

#include "stm32f4xx_hal.h"
#include "string.h"
#include "cmsis_os.h"
#include "driver/uart/drvuart.h"
#include "stdio.h"
#include "service/logging/serlog.h"

#define SERCLI_MAX_COMMAND_LENGTH 256
#define SERCLI_MAX_PARAMETERS 10


typedef enum {
    sercli_OK=0,
    sercli_ERROR,
    sercli_UART_BUSY,
    sercli_TIMEOUT
} sercli_Status;

typedef struct {
    const char *command;
    sercli_Status (*handler)(const uint8_t argc, const char * const argv[]);
    const char *help;
} sercli_CommandStruct;


sercli_Status sercli_Init(drvuart_Handle *drv_handle, sercli_CommandStruct *commands);
sercli_Status sercli_DeInit(void);
sercli_Status sercli_CheckHealth(void);







#endif /* SERCLI_H */