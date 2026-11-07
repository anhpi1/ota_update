#ifndef __SERBACKUP_H
#define __SERBACKUP_H

#include "stm32f4xx_hal.h"
#include "cmsis_os.h"
#include "driver/flash/drvFlash.h"
#include "driver/crc/drvcrc.h"
#include "string.h"
#include "stdint.h"


#define SERBACKUP_MAX_INDEX_TABLE 16 // đơn vị word 32 bit
#define SERBACKUP_MAX_DATA_LENGTH 8 // đơn vị word 32 bit
#define SERBACKUP_MAX_STEP 10 
/*
    Số bước chia đôi tối đa: k = log2(SERBACKUP_SIZE_SECTOR / cua_so_quet)
    cua_so_quet = (4 + SERBACKUP_MAX_DATA_LENGTH*4 + 4) + 8 ; // đơn vị bytes
    4 bytes: header (magic + id_data + length)
    SERBACKUP_MAX_DATA_LENGTH*4 bytes: payload (max data)
    8 bytes: dự phòng (padding)
*/

#define SERBACKUP_MAGIC_START 0xB1
#define SERBACKUP_ADDR_SECTOR 0x0800C000
#define SERBACKUP_NAME_SECTOR 3
#define SERBACKUP_SIZE_SECTOR 0x4000 // 16KB đơn vị bytes


typedef enum {
    serbackup_OK=0,
    serbackup_ERROR,
    serbackup_BUSY,
    serbackup_FULL_ID
} serbackup_Status;

typedef struct {
    uint8_t  magic;     // 0xB1 (nhận diện bản ghi)
    uint8_t  id_data;   // Hỗ trợ tối đa 256 loại ID (0 - 255)
    uint16_t length;    // Độ dài payload
} serbackup_Header;     // Tổng đúng 4 bytes 

serbackup_Status serbackup_Init(void);
serbackup_Status serbackup_DeInit(void);
serbackup_Status serbackup_CheckHealth(void);
serbackup_Status serbackup_SaveCheckpoint(uint8_t id_data, uint32_t *data, uint32_t length_index, uint32_t timeout);
serbackup_Status serbackup_LoadCheckpoint(uint8_t id_data, uint32_t *data, uint32_t length_index, uint32_t timeout);

#endif // __SERBACKUP_H