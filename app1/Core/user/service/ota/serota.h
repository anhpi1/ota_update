// file này chỉ có nghiệm vụ xử lý state machie và package OTA, 
// không có nhiệm vụ định tuyến gói tin
#ifndef SEROTA_H
#define SEROTA_H

#include "stm32f411xe.h"
#include <stdint.h>
#include <string.h>

#define SEROTA_MAX_LENGTH_DATA 128 //đơn vị bytes
#define SEROTA_MAX_LENGTH_DATA_WORDS 32 // = SEROTA_MAX_LENGTH_DATA / 4
#define SEROTA_MAX_PACKET_INDEX 1024 // số lượng gói tin tối đa cho phép trong một lần OTA


/* 1. Trạng thái trả về của hàm C */
typedef enum {
    serota_OK = 0,
    serota_ERROR,
    serota_BUSY
} serota_Status;

typedef enum {
    /* Nhóm phản hồi */
    serota_MSG_ACK = 0,
    serota_MSG_NACK,

    /* Nhóm điều khiển & Dữ liệu */
    serota_MSG_PREPARE_UPDATE,
    serota_MSG_START_UPDATE,
    serota_MSG_DATA,             // Gói chứa dữ liệu ghi Flash
    serota_MSG_END_UPDATE,
    serota_MSG_REBOOT
} serota_MsgID;

/* 4. Mã lỗi chi tiết khi NACK */
typedef enum {
    serota_ERR_NONE = 0,
    serota_ERR_INVALID_VERSION,
    serota_ERR_INVALID_AUTHEN,
    serota_ERR_INVALID_INDEX,
    serota_ERR_FLASH_ERASE_FAIL,
    serota_ERR_FLASH_WRITE_FAIL,
    serota_ERR_VERIFY_HASH_FAIL,
    serota_ERR_STATE_INVALID
} serota_ErrorCode;

/* 5. Trạng thái máy FSM */
typedef enum {
    serota_STATE_IDLE = 0,
    serota_STATE_PREPARE,
    serota_STATE_IN_UPDATE,
    serota_STATE_END,
    serota_STATE_REBOOT
} serota_State;

/* 6. Trạng thái máy FSM phía Master */
typedef enum {
    serota_MASTER_IDLE = 0,
    serota_MASTER_PREPARE,
    serota_MASTER_IN_UPDATE,
    serota_MASTER_END,
    serota_MASTER_REBOOT,
    serota_MASTER_DONE,
    serota_MASTER_FAIL
} serota_MasterState;

typedef struct {
    uint8_t  version;        // Phiên bản giao thức (1)
    uint8_t  type;           // serota_MsgID
    uint16_t length;         // SỐ LƯỢNG PHẦN TỬ WORD (0 - 32 words)
    uint32_t authen;         // Token phiên
    uint32_t pack_index;     // Chỉ số gói tin
} serota_Header;

typedef struct {
    serota_Header header;                         // 12 bytes
    uint32_t data[SEROTA_MAX_LENGTH_DATA_WORDS];  // 32 words
    uint32_t crc;                                 // 4 bytes
} serota_Packet;

typedef struct {
    uint8_t sector_update;
    uint8_t padding[3]; // Đệm để căn chỉnh 4 byte
    uint32_t total_size; // Tổng kích thước của firmware mới
    uint32_t total_packets; // Tổng số gói tin dữ liệu
    uint32_t sha256[8]; // Hash của firmware mới (ví dụ: SHA-256)
} serota_UpdateInfo;

typedef serota_Status (*serota_SendFunc)(const uint8_t *data, uint16_t length, uint32_t timeout);
typedef serota_Status (*serota_ReceiveFunc)(uint8_t *data, uint16_t length, uint32_t timeout);

serota_Status serota_Init(serota_SendFunc send_func, serota_ReceiveFunc receive_func);
serota_Status serota_DeInit(void);
serota_Status serota_CheckHealth(void);
serota_Status serota_ProcessPacket(const serota_Packet *packet, serota_Packet *feedback, uint32_t timeout);

serota_Status serota_MasterBegin(serota_Packet *first_packet);
serota_Status serota_ProcessPacketMaster(const serota_Packet *feedback, serota_Packet *next_packet);

#endif // SEROTA_H