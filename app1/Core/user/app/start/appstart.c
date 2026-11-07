#include "appstart.h"
#include "service/ota/serota.h"
#include <string.h>

drvuart_Handle uartHandleDebug;
drvuart_Handle uartHandleOTA;


//////////////////////////////////////////////////////////////////
// khởi tạo task rtos
//////////////////////////////////////////////////////////////////

static osThreadId_t appstart_OtaTaskHandle = NULL;

static const osThreadAttr_t appstart_OtaTask_attributes = {
    .name = "otaTask",
    .stack_size = 512 * 4, // 2 KB
    .priority = (osPriority_t)osPriorityNormal,
};

static void appstart_OtaLoop(void *argument);

static appstart_Status appstart_StartTasks(void)
{
    appstart_Status status = appstart_OK;
    // Task OTA đã được khởi tạo trong appstart_Init
    if (appstart_OtaTaskHandle == NULL)
    {
        appstart_OtaTaskHandle = osThreadNew(appstart_OtaLoop, NULL, &appstart_OtaTask_attributes);
        if (appstart_OtaTaskHandle == NULL) status = appstart_ERROR;
    }

    return status;
}

static void appstart_OtaLoop(void *argument)
{
    (void)argument;
    uint32_t rx_buffer[sizeof(serota_Packet) / 4];
    uint16_t rx_length = 0;
    serota_Packet feedback;

    for (;;)
    {
        /* Lắng nghe gói tin OTA 144 bytes qua UART2 DMA nội bộ */
        rx_length = 0;
        drvuart_Status stat = drvuart_Receive(&uartHandleOTA, (uint8_t *)rx_buffer, &rx_length, osWaitForever);

        if (stat == drvuart_OK && rx_length == sizeof(serota_Packet))
        {
            memset(&feedback, 0, sizeof(feedback));
            const serota_Packet *pkt = (const serota_Packet *)rx_buffer;

            /* Xử lý gói tin và tạo gói tin phản hồi (ACK / NACK) */
            (void)serota_ProcessPacket(pkt, &feedback, 5000);

            /* Nếu không phải Silent Drop (CRC lỗi) thì gửi phản hồi cho Master */
            if (feedback.header.type == serota_MSG_ACK || feedback.header.type == serota_MSG_NACK)
            {
                drvuart_Transmit(&uartHandleOTA, (const uint8_t *)&feedback, sizeof(serota_Packet), 1000);
            }
        }
        else
        {
            osDelay(10);
        }
    }
}
//////////////////////////////////////////////////////////////////
// khởi tạo dịch vụ
//////////////////////////////////////////////////////////////////



appstart_Status appstart_Init(UART_HandleTypeDef *huart1, UART_HandleTypeDef *huart2, CRC_HandleTypeDef *hcrc, sercli_CommandStruct *cmd_list)
{
    appstart_Status status = appstart_OK;
    if (drvuart_Init(&uartHandleDebug, huart1) != drvuart_OK) status = appstart_ERROR;
    if (drvuart_Init(&uartHandleOTA, huart2) != drvuart_OK) status = appstart_ERROR;
    if (drvflash_Init() != drvflash_OK) status = appstart_ERROR;
    if (drvcrc_Init(hcrc) != drvcrc_OK) status = appstart_ERROR;
    if (serlog_Init(&uartHandleDebug) != serlog_OK) status = appstart_ERROR;
    if (sercli_Init(&uartHandleDebug, cmd_list) != sercli_OK) status = appstart_ERROR;
    if (serbackup_Init() != serbackup_OK) status = appstart_ERROR;

    /* Khởi động luôn Task lắng nghe OTA trên UART2 */
    appstart_StartTasks();
    
    if (status != appstart_OK && appstart_CheckHealth() != appstart_OK) {
        appstart_DeInit(); // Clean up if initialization failed
    }
    return status;
}

appstart_Status appstart_DeInit(void){
    // không tắt dịch vụ log và cli để còn xem lỗi
    appstart_Status status = appstart_OK;
    if (serbackup_DeInit() != serbackup_OK) status = appstart_ERROR;
    // if (sercli_DeInit() != sercli_OK) status = appstart_ERROR;
    // if (serlog_DeInit() != serlog_OK) status = appstart_ERROR; 
    if (drvcrc_DeInit() != drvcrc_OK) status = appstart_ERROR;
    if (drvflash_DeInit() != drvflash_OK) status = appstart_ERROR;
    if (drvuart_DeInit(&uartHandleOTA) != drvuart_OK) status = appstart_ERROR;
    // if (drvuart_DeInit(&uartHandleDebug) != drvuart_OK) status = appstart_ERROR;
    return status;
}
appstart_Status appstart_CheckHealth(void){
    if (drvuart_CheckHealth(&uartHandleDebug) != drvuart_OK) return appstart_ERROR;
    if (drvuart_CheckHealth(&uartHandleOTA) != drvuart_OK) return appstart_ERROR;
    if (drvflash_CheckHealth() != drvflash_OK) return appstart_ERROR;
    if (drvcrc_CheckHealth() != drvcrc_OK) return appstart_ERROR;
    if (serlog_CheckHealth() != serlog_OK) return appstart_ERROR;
    if (sercli_CheckHealth() != sercli_OK) return appstart_ERROR;
    if (serbackup_CheckHealth() != serbackup_OK) return appstart_ERROR;
    return appstart_OK;
}

