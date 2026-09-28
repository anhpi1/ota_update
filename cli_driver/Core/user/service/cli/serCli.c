#include "serCli.h"


static osThreadId_t cli_TaskHandle;
static const osThreadAttr_t cli_Task_attributes = {
  .name = "cli_TaskTransmit",
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
static drvuart_Handle *cli_drv_uart_handle;




static uint8_t cli_SplitString(char *input, char *output[], uint8_t max_parameters);
static void cli_HearTask(void *argument);




static cli_CommandStruct *p_cli_commands; // Pointer to the command array

cli_Status cli_Init(drvuart_Handle *drv_handle, cli_CommandStruct *commands)
{
    cli_Status status = cliS_ok;
   
    if(drv_handle == NULL || commands == NULL) return cliS_error;

    p_cli_commands = commands;
    cli_drv_uart_handle = drv_handle;   
    cli_TaskHandle = osThreadNew(cli_HearTask, NULL, &cli_Task_attributes);

    if (cli_CheckHealth() != cliS_ok) status = cliS_error;

    if(status != cliS_ok) cli_DeInit(); // Clean up if initialization failed

    return status;
}

cli_Status cli_DeInit(void)
{
    cli_drv_uart_handle = NULL;
    if (cli_TaskHandle != NULL) osThreadTerminate(cli_TaskHandle);
    cli_TaskHandle = NULL;
    p_cli_commands = NULL;
    return cliS_ok;
}

cli_Status cli_CheckHealth(void)
{
    if(cli_drv_uart_handle == NULL || cli_TaskHandle == NULL) return cliS_error;
     if( log_CheckHealth() != logS_ok) return cliS_error;
    return cliS_ok;
}

cli_Status cli_HandleCommand(uint8_t argc, const char * const argv[])
{
    if (argc == 0 || argv == NULL) {
        return cliS_error;
    }

    for (cli_CommandStruct *cmd = p_cli_commands; cmd->command != NULL; cmd++) {
        if (strcmp(argv[0], cmd->command) == 0) {
            if (cmd->handler != NULL) {
                cmd->handler(argc, argv); 
                return cliS_ok;
            } else {
                return cliS_error;
            }
        }
    }

    return cliS_error;
}

void cli_HearTask(void *argument)
{
    while (1)
    {
        cli_Status clistatus = cliS_ok;
        drvuart_Status status = DRVUART_OK;
        char cli_buffer[CLI_MAX_COMMAND_LENGTH] = {0};    
        uint16_t cli_buffer_length = 0;
        status = drvuart_Receive(cli_drv_uart_handle, (uint8_t *)cli_buffer, &cli_buffer_length, osWaitForever);
        if (status != DRVUART_OK || cli_buffer_length == 0 ) continue; // Skip to the next iteration if there's an error
        if (cli_buffer_length > CLI_MAX_COMMAND_LENGTH)
        {
            printf("Received command length exceeds maximum allowed length.\r\n");
            continue; // Skip to the next iteration if the command is too long
        }
        printf("\r\n");
        

        char *cli_args[CLI_MAX_PARAMETERS] = {0};
        uint8_t args_count = cli_SplitString(cli_buffer, cli_args, CLI_MAX_PARAMETERS);
        if (args_count == 0) continue; // Bỏ qua nếu chuỗi rỗng hoặc chỉ có khoảng trắng / Enter

        clistatus = cli_HandleCommand(args_count, (const char * const *)cli_args);

        if (clistatus != cliS_ok) printf("Invalid command or error executing command.\r\n");

    }
}




static uint8_t cli_SplitString(char *input, char *output[], uint8_t max_parameters) 
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

