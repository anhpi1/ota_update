/**
  ******************************************************************************
  * @file    log.h
  * @brief   Thư viện ghi Log hệ thống qua DMA Ring Buffer.
  *
  * @note    LƯU Ý KHI SỬ DỤNG:
  *          1. Trình tự gọi ghi log:
  *             - log_init(&Lmsg)       : Khởi tạo struct log toàn cục 1 lần.
  *             - log_add_init(&Lmsg)   : Bắt đầu nạp 1 bản ghi log mới.
  *             - log_add_text / uint.. : Ghép nội dung log mong muốn.
  *             - log_end(...)          : Đóng gói prefix [ERROR/WARN/INFO], file, line, suffix.
  *             - log_trasnmit(&Lmsg)   : Đẩy bản ghi trong hàng đợi ra UART DMA.
  *
  *          2. Trong hàm HAL_UART_TxCpltCallback():
  *             - Cần đặt lại biến `len_tx_buf = 0;` khi DMA hoàn tất truyền.
  *
  *          3. YÊU CẦU KÊNH TRUYỀN:
  *             - Thư viện này yêu cầu KÊNH UART RIÊNG (Dedicated channel).
  *             - KHÔNG ĐƯỢC DÙNG CHUNG kênh UART này với các module hoặc tác vụ khác.
  ******************************************************************************
  */

#ifndef LOG_H
#define LOG_H

#include "main.h"

#define LOG_MAX_LENGTH 128
#define LOG_MAX_QUEUE_LENGTH 16

typedef enum {
    LOG_TYPE_ERROR,
    LOG_TYPE_WARNING,
    LOG_TYPE_INFO
} LogType;

typedef struct {
    char msg[LOG_MAX_QUEUE_LENGTH][LOG_MAX_LENGTH];
    uint8_t msg_len[LOG_MAX_QUEUE_LENGTH];
    uint8_t is_data_ready[LOG_MAX_QUEUE_LENGTH];
    uint8_t point_write;
    uint8_t point_read;
} LogMessage;

// Hàm truyền thực tế ra UART DMA
uint8_t log_trasnmit(LogMessage *Lmsg);

uint8_t log_next_element(uint8_t element);
uint8_t get_filename(const char *path, char *filename);

uint8_t log_init(LogMessage *Lmsg);
uint8_t log_add_init(LogMessage *Lmsg);

// Các hàm helper hỗ trợ ghép chuỗi
uint8_t log_add_text(LogMessage *Lmsg, const char *paraph);
uint8_t log_add_uint8_t(LogMessage *Lmsg, uint8_t value);
uint8_t log_add_uint16_t(LogMessage *Lmsg, uint16_t value);
uint8_t log_add_uint32_t(LogMessage *Lmsg, uint32_t value);

// Hàm kết thúc đóng gói log
uint8_t log_end(uint8_t type, LogMessage *Lmsg, const char *file, uint32_t line);



#endif /* LOG_H */
