#include "log.h"


extern UART_HandleTypeDef huart1;
extern uint8_t uart1_tx_buf[];

uint8_t log_transmit(LogMessage *Lmsg) 
{
    // định nghĩa kiểm tra kênh truyền ở đây
    if(huart1.gState == HAL_UART_STATE_BUSY_TX) return 0; // Nếu đang truyền thì không làm gì
    //kết thúc định nghĩa kiểm tra kênh truyền ở đây

    
    uint8_t lne = log_next_element(Lmsg->point_read);
    if(Lmsg->is_data_ready[lne] == 0) return 0; // Nếu ô đọc chưa sẵn sàng, không làm gì
    Lmsg->point_read = lne;

    
    //định nghia hàm truyền ra ngoài (UART/LoRa/...) ở đây
    uint8_t len_tx_buf = Lmsg->msg_len[Lmsg->point_read];

    for(uint8_t i = 0; i < len_tx_buf; i++)
    {
        uart1_tx_buf[i] = Lmsg->msg[Lmsg->point_read][i];
    }

    HAL_UART_Transmit_DMA(&huart1, (uint8_t*)uart1_tx_buf, len_tx_buf);
    //kết thúc định nghĩa hàm truyền ra ngoài (UART/LoRa/...) ở đây



    for(uint8_t i = 0; i < Lmsg->msg_len[Lmsg->point_read]; i++)
    {
        Lmsg->msg[Lmsg->point_read][i] = 0;
    }
    Lmsg->msg_len[Lmsg->point_read] = 0;
    Lmsg->is_data_ready[Lmsg->point_read] = 0;

    return 1;
}


uint8_t log_next_element(uint8_t element)
{  
    return (element + 1) % LOG_MAX_QUEUE_LENGTH;
}

uint8_t get_filename(const char *path, char *file) {
    if (!path || !file) return 0;

    const char *last_slash = path;
    const char *p = path;

    // Tìm dấu '/' hoặc '\' cuối cùng
    while (*p != '\0') {
        if (*p == '/' || *p == '\\') {
            last_slash = p + 1;
        }
        p++;
    }

    // Chép tên file ngắn gọn vào mảng đích (tối đa 31 ký tự + '\0')
    uint8_t idx = 0;
    while (*last_slash != '\0' && idx < 31) {
        file[idx++] = *last_slash++;
    }
    file[idx] = '\0';

    return 1;
}

uint8_t log_init(LogMessage *Lmsg) {
    if (!Lmsg) return 0;

    for (uint8_t i = 0; i < LOG_MAX_QUEUE_LENGTH; i++) {
        Lmsg->msg_len[i] = 0;
        Lmsg->is_data_ready[i] = 0;
    }
    Lmsg->point_write = 0;
    Lmsg->point_read = 0;

    return 1;
}






uint8_t log_add_init(LogMessage *Lmsg)
{
    uint8_t lne = log_next_element(Lmsg->point_write);
    if(Lmsg->is_data_ready[lne] == 1) return 0; // Nếu ô ghi tiếp theo chưa được đọc, không làm gì
    Lmsg->point_write = lne;
    return 1;
}

uint8_t log_add_text(LogMessage *Lmsg, const char *paraph) {
    if (!Lmsg || !paraph) return 0;

    char *msg = Lmsg->msg[Lmsg->point_write];
    uint8_t *msg_len = &Lmsg->msg_len[Lmsg->point_write];

    if (!msg || !msg_len) return 0;

    while (*paraph != '\0' && *msg_len < (LOG_MAX_LENGTH - 1)) {
        msg[*msg_len] = *paraph++;
        (*msg_len)++;
    }
    msg[*msg_len] = '\0'; // Luôn kết thúc bằng ký tự null

    return 1;
}

// Chuyển đổi số uint32_t sang chuỗi và thêm vào msg
uint8_t log_add_uint32_t(LogMessage *Lmsg, uint32_t value) {
    if (!Lmsg) return 0;

    char *msg = Lmsg->msg[Lmsg->point_write];
    uint8_t *msg_len = &Lmsg->msg_len[Lmsg->point_write];

    if (!msg || !msg_len) return 0;

    // Trường hợp giá trị bằng 0
    if (value == 0) {
        if (*msg_len < (LOG_MAX_LENGTH - 1)) {
            msg[(*msg_len)++] = '0';
            msg[*msg_len] = '\0';
        }
        return 1;
    }

    // Tách từng chữ số từ hàng đơn vị
    char temp[11]; // Tối đa 10 chữ số cho uint32_t + '\0'
    uint8_t count = 0;

    while (value > 0) {
        temp[count++] = (value % 10) + '0';
        value /= 10;
    }

    // Đảo ngược lại và chép vào msg
    while (count > 0 && *msg_len < (LOG_MAX_LENGTH - 1)) {
        msg[(*msg_len)++] = temp[--count];
    }
    msg[*msg_len] = '\0';

    return 1;
}

uint8_t log_add_uint8_t(LogMessage *Lmsg, uint8_t value) {
    return log_add_uint32_t(Lmsg, (uint32_t)value);
}

uint8_t log_add_uint16_t(LogMessage *Lmsg, uint16_t value) {
    return log_add_uint32_t(Lmsg, (uint32_t)value);
}

// -------------------------------------------------------------
// 3. Các hàm log chính
// -------------------------------------------------------------


// Format: [ERROR] file:"..." line:"..." msg:"..."\r\n
uint8_t log_end(uint8_t type, LogMessage *Lmsg, const char *file, uint32_t line) {

    char file_name[32] = {0};
    get_filename(file, file_name);

    char *msg;
    uint8_t *msg_len;

    msg = Lmsg->msg[Lmsg->point_write];
    msg_len = &Lmsg->msg_len[Lmsg->point_write];

    char temp[256] = {0};
    uint8_t temp_len = 0;

   while (msg[temp_len] != '\0' && temp_len < (LOG_MAX_LENGTH - 1)) {
    temp[temp_len] = msg[temp_len];
    temp_len++;
    }
    temp[temp_len] = '\0';

    for (uint8_t i = 0; i < *msg_len; i++) {
        msg[i] = 0;
    }
    *msg_len = 0;
    
    if(type == 0) log_add_text(Lmsg, "[ERROR] ");
    else if(type == 1) log_add_text(Lmsg, "[WARNING] ");
    else if(type == 2) log_add_text(Lmsg, "[INFO] ");
    
    log_add_text(Lmsg, "file:\"");
    log_add_text(Lmsg, file_name);
    log_add_text(Lmsg, "\" line:\"");
    log_add_uint32_t(Lmsg, line);
    log_add_text(Lmsg, "\" msg:\"");
    log_add_text(Lmsg, temp);
    log_add_text(Lmsg, "\"\r\n");
    Lmsg->is_data_ready[Lmsg->point_write] = 1;
    return 1;
}




