#ifndef SERCLI_H
#define SERCLI_H

// cli được thiết kế theo dạng chỉ xử lý 1 lệnh rồi mới nhận lệnh tiếp theo không có hàng đợi thực thi lệnh
// vì nó không cần thiết, tốc độ nhập của con người khá chậm và tốc độ thực thi lệnh cũng khá nhanh


#include "main.h"
#include "string.h"
#include "cmsis_os.h"



//setting
#define CLI_MAX_BUFF_LENGTH 128


typedef enum {
    CLI_OK = 0,
    CLI_ERROR,
    CLI_UART_BUSY
} cli_Status;

typedef struct {
    UART_HandleTypeDef *huart;
    char msg_rx[CLI_MAX_BUFF_LENGTH];
    char msg_tx[CLI_MAX_BUFF_LENGTH];
    uint8_t tx_length;
    uint8_t rx_length;
    uint8_t is_data_rx_ready;
    uint8_t is_data_tx_ready;
} cli_Struct;



cli_Status cli_Init(cli_Struct *Struct, UART_HandleTypeDef *Huart);
cli_Status cli_CallBackReceive(cli_Struct *Struct, uint8_t Size);
cli_Status cli_Receive(cli_Struct *Struct);
cli_Status cli_Transmit(cli_Struct *Struct);
cli_Status cli_HandleCommand(cli_Struct *Struct);



#endif /* SERCLI_H */