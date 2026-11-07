#include "serCli.h"


static osThreadId_t sercli_TaskHandle;
static const osThreadAttr_t sercli_Task_attributes = {
  .name = "sercli_TaskTransmit",
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
static drvuart_Handle *sercli_drv_uart_handle;




static uint8_t sercli_SplitString(char *input, char *output[], uint8_t max_parameters);
static void sercli_HearTask(void *argument);




static sercli_CommandStruct *p_sercli_commands; // Pointer to the command array

sercli_Status sercli_Init(drvuart_Handle *drv_handle, sercli_CommandStruct *commands)
{
    sercli_Status status = sercli_OK;
   
    if(drv_handle == NULL || commands == NULL) return sercli_ERROR;

    p_sercli_commands = commands;
    sercli_drv_uart_handle = drv_handle;   
    sercli_TaskHandle = osThreadNew(sercli_HearTask, NULL, &sercli_Task_attributes);

    if (sercli_CheckHealth() != sercli_OK) status = sercli_ERROR;

    if(status != sercli_OK) sercli_DeInit(); // Clean up if initialization failed

    return status;
}

sercli_Status sercli_DeInit(void)
{
    sercli_drv_uart_handle = NULL;
    if (sercli_TaskHandle != NULL) osThreadTerminate(sercli_TaskHandle);
    sercli_TaskHandle = NULL;
    p_sercli_commands = NULL;
    return sercli_OK;
}

sercli_Status sercli_CheckHealth(void)
{
    if(sercli_drv_uart_handle == NULL || sercli_TaskHandle == NULL || p_sercli_commands == NULL) return sercli_ERROR;
     if( serlog_CheckHealth() != serlog_OK) return sercli_ERROR;
    return sercli_OK;
}

sercli_Status sercli_HandleCommand(uint8_t argc, const char * const argv[])
{
    if (argc == 0 || argv == NULL) {
        return sercli_ERROR;
    }
    if(sercli_CheckHealth() != sercli_OK) return sercli_ERROR;

    for (sercli_CommandStruct *cmd = p_sercli_commands; cmd->command != NULL; cmd++) {
        if (strcmp(argv[0], cmd->command) == 0) {
            if (cmd->handler != NULL) {
                cmd->handler(argc, argv); 
                return sercli_OK;
            } else {
                return sercli_ERROR;
            }
        }
    }

    return sercli_ERROR; // Command not found
}

void sercli_HearTask(void *argument)
{
    while (1)
    {
        sercli_Status serclistatus = sercli_OK;
        drvuart_Status status = drvuart_OK;
        char sercli_buffer[SERCLI_MAX_COMMAND_LENGTH] = {0};    
        uint16_t sercli_buffer_length = 0;
        status = drvuart_Receive(sercli_drv_uart_handle, (uint8_t *)sercli_buffer, &sercli_buffer_length, osWaitForever);
        if (status != drvuart_OK || sercli_buffer_length == 0 ) continue; // Skip to the next iteration if there's an error
        if (sercli_buffer_length > SERCLI_MAX_COMMAND_LENGTH)
        {
            printf("Received command length exceeds maximum allowed length.\r\n");
            continue; // Skip to the next iteration if the command is too long
        }
        printf("\r\n");
        

        char *sercli_args[SERCLI_MAX_PARAMETERS] = {0};
        uint8_t args_count = sercli_SplitString(sercli_buffer, sercli_args, SERCLI_MAX_PARAMETERS);
        if (args_count == 0) continue; // Bỏ qua nếu chuỗi rỗng hoặc chỉ có khoảng trắng / Enter

        serclistatus = sercli_HandleCommand(args_count, (const char * const *)sercli_args);

        if (serclistatus != sercli_OK) printf("Invalid command or error executing command.\r\n");

    }
}




static uint8_t sercli_SplitString(char *input, char *output[], uint8_t max_parameters) 
{   
    // strtok đã phá hủy biến input vì biến input không cần thiết sau khi tách lệnh nên có thể để phá hủy
    // nếu sử dụng dma thì nên copy input sang biến khác để tránh phá hủy input
    uint8_t parameter_count = 0;
    char *token = strtok(input, " \r\n"); // Split by space, carriage return, and newline
    
    while (token != NULL && parameter_count < max_parameters) {
        output[parameter_count++] = token;
        token = strtok(NULL, " \r\n"); // Continue splitting the string
    }
    
    return parameter_count;
}

