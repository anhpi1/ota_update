#ifndef SERCLI_H
#define SERCLI_H

// cli được thiết kế theo dạng chỉ xử lý 1 lệnh rồi mới nhận lệnh tiếp theo không có hàng đợi thực thi lệnh
// vì nó không cần thiết, tốc độ nhập của con người khá chậm và tốc độ thực thi lệnh cũng khá nhanh
// cli phụ thuộc vào log, vì vậy cần khởi tạo log trước khi khởi tạo cli

#include "main.h"
#include "string.h"
#include "cmsis_os.h"
#include "driver/uart/drvUart.h"
#include "stdio.h"
#include "service/logging/serLog.h"

#define CLI_MAX_COMMAND_LENGTH 256
#define CLI_MAX_PARAMETERS 10


typedef enum {
    cliS_ok=0,
    cliS_error,
    cliS_uart_busy,
    cliS_timeout
} cli_Status;

typedef struct {
    const char *command;
    cli_Status (*handler)(const uint8_t argc, const char * const argv[]);
    const char *help;
} cli_CommandStruct;


cli_Status cli_Init();
cli_Status cli_DeInit();
cli_Status cli_CheckHealth();







#endif /* SERCLI_H */