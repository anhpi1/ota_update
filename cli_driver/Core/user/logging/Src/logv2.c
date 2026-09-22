#include "logv2.h"

extern UART_HandleTypeDef huart1;
extern uint8_t uart1_tx_buf[];
extern LogMessage Lmsg;



int _write(int file, char *ptr, int len)
{
    if(!log_add_init()){
        while(!log_add_init()) osDelay(10);
        log_add_text("[ERROR] overflow buffer log", 28);
        Lmsg.is_data_ready[Lmsg.point_write] = 1;
        return 0;
    }
    log_add_text(ptr,len);
    Lmsg.is_data_ready[Lmsg.point_write] = 1;
    return len;
}

uint8_t log_transmit(void) 
{
    // định nghĩa kiểm tra kênh truyền ở đây
    if(huart1.gState == HAL_UART_STATE_BUSY_TX) return 0; // Nếu đang truyền thì không làm gì
    //kết thúc định nghĩa kiểm tra kênh truyền ở đây

    
    uint8_t lne = log_next_element(Lmsg.point_read);
    if(Lmsg.is_data_ready[lne] == 0) return 0; // Nếu ô đọc chưa sẵn sàng, không làm gì
    Lmsg.point_read = lne;

    
    //định nghia hàm truyền ra ngoài (UART/LoRa/...) ở đây
    uint8_t len_tx_buf = Lmsg.msg_len[Lmsg.point_read];

    for(uint8_t i = 0; i < len_tx_buf; i++)
    {
        uart1_tx_buf[i] = Lmsg.msg[Lmsg.point_read][i];
    }

    HAL_UART_Transmit_DMA(&huart1, (uint8_t*)uart1_tx_buf, len_tx_buf);
    //kết thúc định nghĩa hàm truyền ra ngoài (UART/LoRa/...) ở đây




    for(uint8_t i = 0; i < Lmsg.msg_len[Lmsg.point_read]; i++)
    {
        Lmsg.msg[Lmsg.point_read][i] = 0;
    }
    Lmsg.msg_len[Lmsg.point_read] = 0;
    Lmsg.is_data_ready[Lmsg.point_read] = 0;

    return 1;
}

uint8_t log_next_element(uint8_t element)
{
    return (element + 1) % LOG_MAX_QUEUE_LENGTH;
}

const char* get_filename(const char *path) {
    if (!path) return "";

    const char *last = path;
    while (*path != '\0') {
        if (*path == '/' || *path == '\\') {
            last = path + 1;
        }
        path++;
    }
    return last;
}

uint8_t log_init(void) {

    for (uint8_t i = 0; i < LOG_MAX_QUEUE_LENGTH; i++) {
        Lmsg.is_data_ready[i] = 0;
        Lmsg.msg_len[i] = 0;
    }
    Lmsg.point_write = 0;
    Lmsg.point_read = 0;

    return 1;
}

uint8_t log_add_init(void)
{
    uint8_t lne = log_next_element(Lmsg.point_write);
    if(Lmsg.is_data_ready[lne] == 1) return 0; // Nếu ô ghi tiếp theo chưa được đọc, không làm gì
    Lmsg.point_write = lne;
    Lmsg.is_data_ready[lne] = 0;
    return 1;
}



uint8_t log_add_text(const char *paraph,int len) {
    if (!paraph) return 0;

    char *msg = Lmsg.msg[Lmsg.point_write];
    uint8_t *msg_len = &Lmsg.msg_len[Lmsg.point_write];
    
    if (!msg || !msg_len) return 0;

    for(int i = 0; i < len && *msg_len < (LOG_MAX_LENGTH - 1); i++) {
        msg[*msg_len] = paraph[i];
        (*msg_len)++;
    }

    msg[*msg_len] = '\0'; // Luôn kết thúc bằng ký tự null

    return 1;
}

