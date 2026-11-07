#include "serota.h"
#include "driver/crc/drvCrc.h"
#include "driver/flash/drvflash.h"
#include "service/ota/serota.h"
#include "stm32f4xx_hal.h"
#include "driver/crc/drvcrc.h"
#include "driver/uart/drvuart.h"
#include "driver/flash/drvFlash.h"
#include "service/logging/serlog.h"
#include "bootloader/sha/sha256.h"
#include <string.h>
#include "bootloader/bootloader.h"



static serota_SendFunc serota_Send = NULL;
static serota_ReceiveFunc serota_Receive = NULL;

static uint8_t serota_machine_state = serota_STATE_IDLE;
static serota_UpdateInfo serota_update_info;
static uint32_t serota_index_packet = 0; // Địa chỉ hiện tại trong Flash để ghi dữ liệu OTA

/* ================= Quản lý phiên Master ================= */
static uint8_t            serota_master_state       = serota_MASTER_IDLE;
static serota_UpdateInfo  serota_master_update_info;
static uint32_t           serota_master_index       = 0;
static const uint32_t    *serota_master_src_addr    = NULL;



serota_Status serota_Init(serota_SendFunc send_func, serota_ReceiveFunc receive_func)
{
    serota_Status status = serota_OK;

    if (send_func == NULL || receive_func == NULL) {
        return serota_ERROR;
    }

    serota_Send = send_func;
    serota_Receive = receive_func;



    if (status != serota_OK && serota_CheckHealth() != serota_OK) {
        serota_DeInit(); // Clean up if initialization failed
    }
    return status;
}

serota_Status serota_DeInit(void)
{
    serota_Status status = serota_OK;
    serota_Send = NULL;
    serota_Receive = NULL;
    return status;
}

serota_Status serota_CheckHealth(void)
{
    if (drvflash_CheckHealth() != drvflash_OK) return serota_ERROR;
    if (drvcrc_CheckHealth() != drvcrc_OK) return serota_ERROR;
    if (serlog_CheckHealth() != serlog_OK) return serota_ERROR;
    return serota_OK;
}


serota_Status serota_ProcessEndUpdate(uint32_t timeout)
{
    serota_Status status = serota_OK;

    if (serota_machine_state != serota_STATE_IN_UPDATE) return serota_ERROR;
    if (serota_index_packet != serota_update_info.total_packets) return serota_ERROR;
    drvflash_Lock(serota_update_info.sector_update);

    drvflash_SectorInfo s_info;
    if(drvflash_GetSectorInfo(serota_update_info.sector_update, &s_info) != drvflash_OK) return serota_ERROR;

    BYTE calculated_hash[32];
    SHA256_CTX ctx;
    sha256_init(&ctx);
    sha256_update(&ctx, (const BYTE *)s_info.address, serota_update_info.total_size);
    sha256_final(&ctx, calculated_hash);

    if (memcmp(calculated_hash, serota_update_info.sha256, 32) != 0) 
    {
        serota_machine_state = serota_STATE_IDLE; // Sai hash -> Hủy phiên
        return serota_ERROR;
    }

    serota_machine_state = serota_STATE_END;
    return status;
}

serota_Status serota_ProcessReboot(uint32_t timeout)
{
    serota_Status status = serota_OK;

    if (serota_CheckHealth() != serota_OK) return serota_ERROR;
    if (serota_machine_state != serota_STATE_END) return serota_ERROR;

    /* 1. Chuẩn bị buffer Metadata 40 bytes (10 Words) căn hàng 4-byte */
    uint32_t metadata_buf[10];
    memset(metadata_buf, 0xFF, sizeof(metadata_buf));

    /* 32 bytes đầu: SHA-256 */
    memcpy(metadata_buf, serota_update_info.sha256, 32);

    /* offset 32: boot_count (đã memset 0xFF) */
    /* offset 33: state = STATE_TESTING (0xFD) */
    uint8_t *p_state = (uint8_t *)metadata_buf + 33;
    *p_state = STATE_TESTING;

    /* offset 36: image_size (uint32_t) */
    uint32_t *p_size = (uint32_t *)((uint8_t *)metadata_buf + 36);
    *p_size = serota_update_info.total_size;

    /* 2. Xác định Sector và địa chỉ ghi Metadata */
    uint8_t meta_sector = 0;
    uint32_t target_addr = 0;
    if (serota_update_info.sector_update == 5) {
        meta_sector = 1;
        target_addr = ADDR_APP1_INFO;
    } else if (serota_update_info.sector_update == 6) {
        meta_sector = 2;
        target_addr = ADDR_APP2_INFO;
    } else {
        return serota_ERROR;
    }

    /* 3. Xóa trắng và ghi Metadata mới vào đúng Sector tương ứng */
    if (drvflash_Unlock(meta_sector, timeout) != drvflash_OK) return serota_ERROR;
    if (drvflash_Erase(meta_sector, timeout) != drvflash_OK) {
        drvflash_Lock(meta_sector);
        return serota_ERROR;
    }
    drvflash_SetWritePtr(meta_sector, target_addr);
    if (drvflash_Write(meta_sector, (uint8_t *)metadata_buf, sizeof(metadata_buf), timeout) != drvflash_OK) {
        drvflash_Lock(meta_sector);
        return serota_ERROR;
    }
    drvflash_Lock(meta_sector);

    /* 4. Chuyển FSM và kích hoạt System Reset */
    serota_machine_state = serota_STATE_REBOOT;

    /* Delay ngắn để xả hết UART buffer trước khi CPU bị reset */
        
    for(volatile int i = 0; i < 1000000; i++); // Delay đơn giản, có thể thay bằng HAL_Delay nếu cần


    __disable_irq();
    NVIC_SystemReset();

    return status;
}

serota_Status serota_ProcessUpdate(const uint32_t *data, uint16_t length, uint32_t index, uint32_t timeout)
{
    serota_Status status = serota_OK;
    if (data == NULL || length == 0) return serota_ERROR;
    if (serota_CheckHealth() != serota_OK) return serota_ERROR;


    if (serota_machine_state == serota_STATE_PREPARE) {
        if (index != 1) return serota_ERROR; // Gói đầu tiên bắt buộc phải là 1
    } else if (serota_machine_state == serota_STATE_IN_UPDATE) {
        if (index != (serota_index_packet + 1)) return serota_ERROR; // Các gói sau phải nối tiếp
    } else return serota_ERROR;
    
    if (drvflash_Write(serota_update_info.sector_update, (uint8_t *)data, length * 4, timeout) != drvflash_OK) return serota_ERROR;
    
    serota_index_packet = index;
    serota_machine_state = serota_STATE_IN_UPDATE;

    return status;
}

serota_Status serota_ProcessPrepareUpdate(const uint32_t *data, uint16_t length, uint32_t timeout)
{
    serota_Status status = serota_OK;
    if (data == NULL || length != (sizeof(serota_UpdateInfo)/4)) return serota_ERROR;
    if (serota_CheckHealth() != serota_OK) return serota_ERROR;
    if (serota_machine_state != serota_STATE_IDLE) return serota_ERROR;
    memset(&serota_update_info, 0, sizeof(serota_update_info));
    memcpy(&serota_update_info, data, sizeof(serota_update_info));

    if(serota_update_info.sector_update < 5 ) return serota_ERROR;
    if(serota_update_info.total_packets > SEROTA_MAX_PACKET_INDEX) return serota_ERROR;
    if(serota_update_info.total_packets == 0) return serota_ERROR;
    drvflash_SectorInfo s_info;
    if(drvflash_GetSectorInfo(serota_update_info.sector_update, &s_info) != drvflash_OK) return serota_ERROR;

    if(serota_update_info.total_size > s_info.size) return serota_ERROR;
    if(serota_update_info.total_size == 0) return serota_ERROR;
    if(serota_update_info.total_size % 4 != 0) return serota_ERROR; 

    drvflash_Unlock(serota_update_info.sector_update, timeout);
    drvflash_Erase(serota_update_info.sector_update, timeout);

    /* Xóa sạch Sector Metadata của Slot đích về 0xFF (STATE_ERASED) để chống rủi ro mất điện giữa chừng */
    uint8_t meta_sector = (serota_update_info.sector_update == 5) ? 1 : 2;
    if (drvflash_Unlock(meta_sector, timeout) == drvflash_OK) {
        drvflash_Erase(meta_sector, timeout);
        drvflash_Lock(meta_sector);
    }

    serota_machine_state = serota_STATE_PREPARE;
    serota_index_packet = 0;
    return status;
}

serota_Status serota_ProcessPacket(const serota_Packet *packet, serota_Packet *feedback, uint32_t timeout)
{
    serota_Status status = serota_OK;
    if (packet == NULL) return serota_ERROR;
    if (packet->header.version != 1) return serota_ERROR;
    if (serota_CheckHealth() != serota_OK) return serota_ERROR;
    if (packet->header.length > SEROTA_MAX_LENGTH_DATA_WORDS) return serota_ERROR;
    if (feedback == NULL) return serota_ERROR;

    if (packet->header.length > 0){

        uint32_t serota_crc = 0;
        
        if (drvcrc_CaculateCRC(packet->data, packet->header.length, &serota_crc, timeout) != drvcrc_OK) return serota_ERROR;
    
        if(serota_crc != packet->crc) status = serota_ERROR;
        else {
            switch (packet->header.type) {
                case serota_MSG_PREPARE_UPDATE:
                    status = serota_ProcessPrepareUpdate(packet->data, packet->header.length, timeout);
                    break;
                case serota_MSG_DATA:
                    status = serota_ProcessUpdate(packet->data, packet->header.length, packet->header.pack_index, timeout);
                    break;
                default:
                    status = serota_ERROR;
                    break;
            }
        }
    } else {
        switch (packet->header.type) {
            case serota_MSG_END_UPDATE:
                status = serota_ProcessEndUpdate(timeout);
                break;
            case serota_MSG_REBOOT:
                status = serota_ProcessReboot(timeout);
                break;
            default:
                status = serota_ERROR;
                break;
        }
    }

    /* Nếu có bất kỳ lỗi nào xảy ra (sai gói, sai CRC, sai thứ tự gói, lỗi Flash...),
       lập tức hủy phiên quay về STATE_IDLE và khóa Flash để tránh treo phiên */
    if (status != serota_OK) {
        if (serota_machine_state == serota_STATE_PREPARE || serota_machine_state == serota_STATE_IN_UPDATE) {
            drvflash_Lock(serota_update_info.sector_update);
        }
        serota_machine_state = serota_STATE_IDLE;
        serota_index_packet = 0;
    }

    feedback->header.version    = 1;
    feedback->header.type       = (status == serota_OK) ? serota_MSG_ACK: serota_MSG_NACK;
    feedback->header.length     = 0;
    feedback->header.authen     = packet->header.authen;
    feedback->header.pack_index = packet->header.pack_index;
    feedback->crc               = 0; // hoặc CRC nếu quy ước


    return status;
}

/* =========================================================================
 * CÁC HÀM NỘI BỘ SINH GÓI TIN MASTER (STATIC)
 * ========================================================================= */
static serota_Status serota_MasterProcessPrepare(serota_Packet *next_packet)
{
    if (next_packet == NULL) return serota_ERROR;
    if (serota_master_state != serota_MASTER_IDLE) return serota_ERROR;

    memset(next_packet, 0, sizeof(serota_Packet));
    next_packet->header.version    = 1;
    next_packet->header.type       = serota_MSG_PREPARE_UPDATE;
    next_packet->header.length     = sizeof(serota_UpdateInfo) / 4; // 11 words
    next_packet->header.authen     = 0x55AA0001;
    next_packet->header.pack_index = 0;

    memcpy(next_packet->data, &serota_master_update_info, sizeof(serota_UpdateInfo));

    if (drvcrc_CaculateCRC(next_packet->data, next_packet->header.length, &next_packet->crc, 100) != drvcrc_OK) {
        return serota_ERROR;
    }

    serota_master_state = serota_MASTER_PREPARE;
    serota_master_index = 0;

    return serota_BUSY;
}

/* =========================================================================
 * CÁC HÀM XỬ LÝ PHÍA MASTER
 * ========================================================================= */
serota_Status serota_MasterBegin(serota_Packet *first_packet)
{
    if (first_packet == NULL) return serota_ERROR;
    if (serota_CheckHealth() != serota_OK) return serota_ERROR;

    /* 1. Kiểm tra trực tiếp bảng Vector trên RAM (0x20000004) để xác định Slot đang chạy */
    uint32_t reset_handler = *(volatile uint32_t *)0x20000004;
    uint32_t meta_addr = 0;

    if (reset_handler >= 0x08020000 && reset_handler < 0x08040000) {
        /* Đang chạy Slot 1 -> Firmware được build cho Slot 1 -> Yêu cầu Slave nạp vào Slot 1 (Sector 5) */
        serota_master_src_addr = (const uint32_t *)0x08020000;
        meta_addr = ADDR_APP1_INFO;
        serota_master_update_info.sector_update = 5;
    } else if (reset_handler >= 0x08040000 && reset_handler < 0x08060000) {
        /* Đang chạy Slot 2 -> Firmware được build cho Slot 2 -> Yêu cầu Slave nạp vào Slot 2 (Sector 6) */
        serota_master_src_addr = (const uint32_t *)0x08040000;
        meta_addr = ADDR_APP2_INFO;
        serota_master_update_info.sector_update = 6;
    } else {
        return serota_ERROR; // Địa chỉ ngoài vùng Flash App
    }

    /* 2. Lấy nguyên vẹn SHA-256 gốc và image_size từ Metadata của chính mình */
    memcpy(serota_master_update_info.sha256, (const void *)meta_addr, 32);
    serota_master_update_info.total_size = *(const uint32_t *)((const uint8_t *)meta_addr + 36);

    /* Kiểm tra tính hợp lệ cơ bản của firmware nguồn */
    if (serota_master_update_info.total_size == 0 || 
        serota_master_update_info.total_size > (128 * 1024) ||
        serota_master_update_info.total_size % 4 != 0) {
        return serota_ERROR;
    }

    /* 3. Tính tổng số gói DATA */
    serota_master_update_info.total_packets = 
        (serota_master_update_info.total_size + (SEROTA_MAX_LENGTH_DATA - 1)) / SEROTA_MAX_LENGTH_DATA;

    /* 4. Sinh gói PREPARE_UPDATE đầu tiên để nhả cho UART truyền đi */
    if (serota_MasterProcessPrepare(first_packet) != serota_BUSY) {
        return serota_ERROR;
    }

    return serota_OK;
}

static serota_Status serota_MasterProcessData(serota_Packet *next_packet)
{
    if (next_packet == NULL) return serota_ERROR;
    if (serota_master_state != serota_MASTER_PREPARE && serota_master_state != serota_MASTER_IN_UPDATE) {
        return serota_ERROR;
    }

    serota_master_index++;
    if (serota_master_index > serota_master_update_info.total_packets) {
        return serota_ERROR;
    }

    uint32_t offset = (serota_master_index - 1) * SEROTA_MAX_LENGTH_DATA;
    uint32_t remain = serota_master_update_info.total_size - offset;
    uint16_t words  = (remain >= SEROTA_MAX_LENGTH_DATA) ? SEROTA_MAX_LENGTH_DATA_WORDS : (remain / 4);

    memset(next_packet, 0, sizeof(serota_Packet));
    next_packet->header.version    = 1;
    next_packet->header.type       = serota_MSG_DATA;
    next_packet->header.length     = words;
    next_packet->header.authen     = 0x55AA0001;
    next_packet->header.pack_index = serota_master_index;

    const uint32_t *p_src = (const uint32_t *)((const uint8_t *)serota_master_src_addr + offset);
    for (uint16_t i = 0; i < words; i++) {
        next_packet->data[i] = p_src[i];
    }
    for (uint16_t i = words; i < SEROTA_MAX_LENGTH_DATA_WORDS; i++) {
        next_packet->data[i] = 0xFFFFFFFF; // Đệm 0xFF cho các word dư của gói cuối
    }

    if (drvcrc_CaculateCRC(next_packet->data, words, &next_packet->crc, 100) != drvcrc_OK) {
        return serota_ERROR;
    }

    serota_master_state = serota_MASTER_IN_UPDATE;
    return serota_BUSY;
}

static serota_Status serota_MasterProcessEnd(serota_Packet *next_packet)
{
    if (next_packet == NULL) return serota_ERROR;
    if (serota_master_state != serota_MASTER_IN_UPDATE) return serota_ERROR;
    if (serota_master_index != serota_master_update_info.total_packets) return serota_ERROR;

    memset(next_packet, 0, sizeof(serota_Packet));
    next_packet->header.version    = 1;
    next_packet->header.type       = serota_MSG_END_UPDATE;
    next_packet->header.length     = 0;
    next_packet->header.authen     = 0x55AA0001;
    next_packet->header.pack_index = serota_master_update_info.total_packets;
    next_packet->crc               = 0;

    serota_master_state = serota_MASTER_END;
    return serota_BUSY;
}

static serota_Status serota_MasterProcessReboot(serota_Packet *next_packet)
{
    if (next_packet == NULL) return serota_ERROR;
    if (serota_master_state != serota_MASTER_END) return serota_ERROR;

    memset(next_packet, 0, sizeof(serota_Packet));
    next_packet->header.version    = 1;
    next_packet->header.type       = serota_MSG_REBOOT;
    next_packet->header.length     = 0;
    next_packet->header.authen     = 0x55AA0001;
    next_packet->header.pack_index = 0;
    next_packet->crc               = 0;

    serota_master_state = serota_MASTER_REBOOT;
    return serota_BUSY;
}

/* =========================================================================
 * HÀM ĐIỀU PHỐI FSM CHÍNH CHO MASTER
 * ========================================================================= */
serota_Status serota_ProcessPacketMaster(const serota_Packet *feedback, serota_Packet *next_packet)
{
    serota_Status status = serota_OK;

    if (next_packet == NULL) {
        status = serota_ERROR;
    }
    /* 1. Xử lý bước REBOOT: Slave đã reboot (hoặc trả ACK) -> Hoàn tất phiên */
    else if (serota_master_state == serota_MASTER_REBOOT) {
        if (feedback != NULL && feedback->header.type == serota_MSG_NACK) {
            status = serota_ERROR;
        } else {
            serota_master_state = serota_MASTER_IDLE; // Xong phiên, đưa về IDLE
            serota_master_index = 0;
            return serota_OK;
        }
    }
    /* 2. Với các bước trước đó: Nếu timeout (feedback == NULL) hoặc nhận NACK -> Báo lỗi */
    else if (feedback == NULL || feedback->header.type != serota_MSG_ACK) {
        status = serota_ERROR;
    }
    /* 3. Phân luồng Dispatcher gọi hàm con tương ứng */
    else {
        switch (serota_master_state) {
            case serota_MASTER_PREPARE:
                status = serota_MasterProcessData(next_packet);
                break;

            case serota_MASTER_IN_UPDATE:
                if (serota_master_index < serota_master_update_info.total_packets) {
                    status = serota_MasterProcessData(next_packet);
                } else {
                    status = serota_MasterProcessEnd(next_packet);
                }
                break;

            case serota_MASTER_END:
                status = serota_MasterProcessReboot(next_packet);
                break;

            default:
                status = serota_ERROR;
                break;
        }
    }

    /* 4. CỬA THOÁT HIỂM TẬP TRUNG: Có lỗi thì reset FSM Master về IDLE */
    if (status == serota_ERROR) {
        serota_master_state = serota_MASTER_IDLE;
        serota_master_index = 0;
    }

    return status;
}


