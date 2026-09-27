#include "serLog.h"


static volatile uint16_t log_queue_full_count = 0;
static volatile uint16_t log_queue_error_count = 0;

static UART_HandleTypeDef *log_uart;


static osMessageQueueId_t log_queue;
static osThreadId_t log_TaskHandle;
static const osThreadAttr_t log_Task_attributes = {
  .name = "log_TaskTransmit",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};



void log_TaskTransmit(void *argument);


int _write(int file, char *ptr, int len)
{
     if (log_CheckHealth() != logS_ok) return 0;
     if (ptr == NULL || len <= 0) return 0;

    log_Element log_msg = {0};
    log_msg.length = (len < (LOG_MAX_LENGTH - 1)) ? (uint8_t)len : (uint8_t)(LOG_MAX_LENGTH - 1);
    memcpy(log_msg.buff, ptr, log_msg.length);
    log_msg.buff[log_msg.length] = '\0'; // Null-terminate the string
    osStatus_t stat = osMessageQueuePut(log_queue, &log_msg, 0, LOG_TIMEOUT);

    if (stat == osOK) return len;

    if (stat == osErrorTimeout) {
        log_queue_full_count++;
        return 0; // Indicate that the write operation failed
    }
    log_queue_error_count++;

    return 0; // Indicate that the write operation failed
}


void log_TaskTransmit(void *argument)
{
    if(log_CheckHealth() != logS_ok) osThreadExit(); // Check if the log system is initialized properly
    while (1) {
        log_Element log_buffer;
        osStatus_t stat = osMessageQueueGet(log_queue, &log_buffer, NULL, osWaitForever);
        if (stat == osOK) {
            if (log_uart != NULL) {
                drvuart_Transmit((uint8_t *)log_buffer.buff, log_buffer.length, LOG_TIMEOUT);
            }
        }
    }
}




const char* get_filename(const char *path) {
    const char *name;

    if (!path) return NULL;

    name = path;
    for (const char *p = path; *p; ++p)
        if (*p == '/' || *p == '\\') name = p + 1;

    return name;
}

log_Status log_Init(UART_HandleTypeDef *huart)
{
    log_Status status = logS_ok;
    if(huart == NULL) return logS_error;
    log_uart = huart;

    log_queue = osMessageQueueNew(LOG_MAX_QUEUE_LENGTH, sizeof(log_Element), NULL);
    log_TaskHandle = osThreadNew(log_TaskTransmit, NULL, &log_Task_attributes);

    if (log_CheckHealth() != logS_ok) log_DeInit();
    
    return status;
}

log_Status log_DeInit(void)
{
    log_Status status = logS_ok;
    osStatus_t stat = osOK;
    if (log_TaskHandle != NULL) {
        stat = osThreadTerminate(log_TaskHandle);
        log_TaskHandle = NULL;
    }

    if (log_queue != NULL) {
        stat = osMessageQueueDelete(log_queue);
        log_queue = NULL;
    }

    if (stat != osOK) status = logS_error;

    return status;
}

log_Status log_CheckHealth(void)
{
    if (log_queue == NULL || log_TaskHandle == NULL) return logS_error;
    return logS_ok;
}

#if LOG_STACK_CHECK
    uint32_t log_GetTaskStackSpace(void)
    {
        if (log_TaskHandle == NULL) return 0;
        return osThreadGetStackSpace(log_TaskHandle);
    }
#endif