#include "serCli.h"




cli_Status cli_Init(cli_Struct *Struct, UART_HandleTypeDef *Huart)
{
    if (Struct == NULL || Huart == NULL) return CLI_ERROR;
    

    Struct->huart = Huart;

    Struct->rx_length = 0;
    Struct->tx_length = 0;
    Struct->is_data_rx_ready = 0;
    Struct->is_data_tx_ready = 0;

    return CLI_OK;
}

cli_Status cli_Receive(cli_Struct *Struct)
{
    if (Struct == NULL) return CLI_ERROR;
    if (Struct->huart->RxState != HAL_UART_STATE_READY) return CLI_UART_BUSY; // UART is busy
    Struct->is_data_rx_ready = 0; 
    HAL_UARTEx_ReceiveToIdle_DMA(Struct->huart, (uint8_t *)Struct->msg_rx, CLI_MAX_BUFF_LENGTH);
    return CLI_OK;
}

cli_Status cli_CallBackReceive(cli_Struct *Struct, uint8_t Size)
{
    if (Struct == NULL) return CLI_ERROR;
    Struct->is_data_rx_ready = 1;
    Struct->rx_length = Size;
    Struct->msg_rx[Size] = '\0';
    return CLI_OK;
}

cli_Status cli_Transmit(cli_Struct *Struct)
{
    if (Struct == NULL) return CLI_ERROR;
    if (Struct->huart->gState != HAL_UART_STATE_READY) return CLI_UART_BUSY; // UART is busy
    Struct->is_data_tx_ready = 0; 
    HAL_UART_Transmit_DMA(Struct->huart, (uint8_t *)Struct->msg_tx, Struct->tx_length);
    return CLI_OK;
}


cli_Status cli_HandleCommand(cli_Struct *Struct)
{
    if (Struct == NULL) return CLI_ERROR;
    return CLI_OK;
}


uint8_t CLI_split_string(char *input, char *output[], uint16_t max_parameters)
{   
    // strtok đã phá hủy biến input vì biến input không cần thiết sau khi tách lệnh nên có thể để phá hủy
    // nếu sử dụng dma thì nên copy input sang biến khác để tránh phá hủy input
    uint16_t parameter_count = 0;
    char *token = strtok(input, " ");
    
    while (token != NULL && parameter_count < max_parameters) {
        output[parameter_count++] = token;
        token = strtok(NULL, " ");
    }
    
    return parameter_count;
}