#include "startup.h"

drvuart_Handle huart1_handle;

cli_CommandStruct sp_cli_commands[] = 
{
    {"help", cli_cmd_Help, "Show available commands"},
    {"version", cli_cmd_Version, "Show version information"},
    {NULL, NULL, NULL}
};

void startup_Init(UART_HandleTypeDef *huart)
{
    drvuart_Init(&huart1_handle, huart);
    log_Init(&huart1_handle);
    cli_Init(&huart1_handle, sp_cli_commands);
    TESTLOG();
}


cli_Status cli_cmd_Help(const uint8_t argc, const char * const argv[]) 
{
    printf("Available commands:\n");
    for (cli_CommandStruct *cmd = sp_cli_commands; cmd->command != NULL; cmd++) {
        printf("  %s: %s\n", cmd->command, cmd->help);
    }
    return cliS_ok;
}

cli_Status cli_cmd_Version(const uint8_t argc, const char * const argv[])
{
    printf("CLI Version: 1.0.0\n");
    return cliS_ok;
}