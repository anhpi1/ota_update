#ifndef STARTUP_H
#define STARTUP_H

#include "main.h"
#include "cmsis_os.h"
#include "service/logging/serLog.h"
#include "tester/testLog.h"
#include "driver/uart/drvUart.h"
#include "service/cli/serCli.h"

void startup_Init(UART_HandleTypeDef *huart);



cli_Status cli_cmd_Help(const uint8_t argc, const char * const argv[]);
cli_Status cli_cmd_Version(const uint8_t argc, const char * const argv[]);




#endif /* STARTUP_H */