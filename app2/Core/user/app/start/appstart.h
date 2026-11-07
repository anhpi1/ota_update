#include "main.h"


#include "cmsis_os.h"



#include "driver/uart/drvUart.h"
#include "driver/flash/drvFlash.h"
#include "driver/crc/drvCrc.h"


#include "service/cli/serCli.h"
#include "service/logging/serLog.h"
#include "service/backup/serBackup.h"

#include "stm32f4xx_hal_uart.h"




typedef enum {
    appstart_OK = 0,
    appstart_ERROR,
    appstart_BUSY
} appstart_Status;

appstart_Status appstart_Init(UART_HandleTypeDef *huart1, UART_HandleTypeDef *huart2, CRC_HandleTypeDef *hcrc, sercli_CommandStruct *cmd_list);
appstart_Status appstart_DeInit(void);
appstart_Status appstart_CheckHealth(void);

/* Khai báo nguyên mẫu (prototypes) từ main.c và freertos.c */
void MX_FREERTOS_Init(void); 
void SystemClock_Config(void);