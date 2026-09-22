#ifndef LOGV2_H
#define LOGV2_H

#include "main.h"
#include "stdio.h"
#include "cmsis_os.h"

#define LOG_MAX_LENGTH 128
#define LOG_MAX_QUEUE_LENGTH 16


typedef struct {
    char msg[LOG_MAX_QUEUE_LENGTH][LOG_MAX_LENGTH];
    uint8_t msg_len[LOG_MAX_QUEUE_LENGTH];
    uint8_t is_data_ready[LOG_MAX_QUEUE_LENGTH];
    uint8_t point_write;
    uint8_t point_read;
} LogMessage;

// Hàm truyền thực tế ra UART DMA
uint8_t log_transmit(void);
uint8_t log_next_element(uint8_t element);
const char* get_filename(const char *path);

uint8_t log_init(void);
uint8_t log_add_init(void);
uint8_t log_add_text(const char *paraph,int len);

#define LOG_ERROR(fmt, ...)   printf("[ERROR] %s:%d: " fmt "\r\n", get_filename(__FILE__), __LINE__, ##__VA_ARGS__)
#define LOG_WARNING(fmt, ...) printf("[WARN]  %s:%d: " fmt "\r\n", get_filename(__FILE__), __LINE__, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...)    printf("[INFO]  %s:%d: " fmt "\r\n", get_filename(__FILE__), __LINE__, ##__VA_ARGS__)


#endif /* LOGV2_H */