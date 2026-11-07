#include "serlog.h"


static volatile uint16_t serlog_queue_full_count = 0;
static volatile uint16_t serlog_queue_error_count = 0;

static drvuart_Handle *serlog_handle;


static osMessageQueueId_t serlog_queue;
static osThreadId_t serlog_TaskHandle;
static const osThreadAttr_t serlog_Task_attributes = {
  .name = "serlog_TaskTransmit",
  .stack_size = 117 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};



void serlog_TaskTransmit(void *argument);


int _write(int file, char *ptr, int len)
{
     if (serlog_CheckHealth() != serlog_OK) return 0;
     if (ptr == NULL || len <= 0) return 0;

    serlog_Element serlog_msg = {0};
    serlog_msg.length = (len < (SERLOG_MAX_LENGTH - 1)) ? (uint8_t)len : (uint8_t)(SERLOG_MAX_LENGTH - 1);
    memcpy(serlog_msg.buff, ptr, serlog_msg.length);
    serlog_msg.buff[serlog_msg.length] = '\0'; // Null-terminate the string
    osStatus_t stat = osMessageQueuePut(serlog_queue, &serlog_msg, 0, SERLOG_TIMEOUT);

    if (stat == osOK) return len;

    if (stat == osErrorTimeout) {
        serlog_queue_full_count++;
        return 0; // Indicate that the write operation failed
    }
    serlog_queue_error_count++;

    return 0; // Indicate that the write operation failed
}


void serlog_TaskTransmit(void *argument)
{
    if(serlog_CheckHealth() != serlog_OK) osThreadExit(); // Check if the serlog system is initialized properly
    while (1) {
        serlog_Element serlog_buffer;
        osStatus_t stat = osMessageQueueGet(serlog_queue, &serlog_buffer, NULL, osWaitForever);
        if (stat == osOK) {
            if (serlog_handle != NULL) {
                drvuart_Transmit(serlog_handle, (uint8_t *)serlog_buffer.buff, serlog_buffer.length, SERLOG_TIMEOUT);
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

serlog_Status serlog_Init(drvuart_Handle *handle)
{
    serlog_Status status = serlog_OK;
    if(handle == NULL) return serlog_ERROR;

    serlog_handle = handle;

    serlog_queue = osMessageQueueNew(SERLOG_MAX_QUEUE_LENGTH, sizeof(serlog_Element), NULL);
    serlog_TaskHandle = osThreadNew(serlog_TaskTransmit, NULL, &serlog_Task_attributes);

    if (serlog_CheckHealth() != serlog_OK) status = serlog_ERROR;
    
    if(status != serlog_OK) serlog_DeInit(); // Clean up if initialization failed

    return status;
}

serlog_Status serlog_DeInit(void)
{
    serlog_Status status = serlog_OK;
    osStatus_t stat = osOK;
    if (serlog_TaskHandle != NULL) {
        stat = osThreadTerminate(serlog_TaskHandle);
        serlog_TaskHandle = NULL;
    }

    if (serlog_queue != NULL) {
        stat = osMessageQueueDelete(serlog_queue);
        serlog_queue = NULL;
    }

    if (stat != osOK) status = serlog_ERROR;

    return status;
}

serlog_Status serlog_CheckHealth(void)
{
    if (serlog_queue == NULL || serlog_TaskHandle == NULL || serlog_handle == NULL) return serlog_ERROR;
    return serlog_OK;
}

#if serlog_STACK_CHECK
    uint32_t serlog_GetTaskStackSpace(void)
    {
        if (serlog_TaskHandle == NULL) return 0;
        return osThreadGetStackSpace(serlog_TaskHandle);
    }
#endif