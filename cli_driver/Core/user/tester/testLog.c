#include "testLog.h"

static osThreadId_t testLog_TaskHandle;
static const osThreadAttr_t testLog_Task_attributes = {
  .name = "testLog_TaskTransmit",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
void testLog_TaskTransmit(void *argument);


void testLogInit(void) {
    testLog_TaskHandle = osThreadNew(testLog_TaskTransmit, NULL, &testLog_Task_attributes);
}


void testLog_TaskTransmit(void *argument) {
    while (1) {

        #if LOG_STACK_CHECK
            uint32_t free_stack = (uint32_t)log_GetTaskStackSpace()/4;
            LOG(I, "Free stack space: %lu bytes", free_stack);
        #else
            LOG(I, "Test log message: %d", 123);
        #endif
        osDelay(1000); // Delay for 1 second
    }
}