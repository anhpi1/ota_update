#ifndef DRVFLASH_H
#define DRVFLASH_H


#include "cmsis_os.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_flash.h"

// lưu ý có thể ghi đè lên ô nhớ đang chứa dữ liệu, cần kiểm tra trước khi ghi

#define DRVFLASH_MAX_SECTORS 8



typedef enum {
    drvflash_OK = 0,
    drvflash_ERROR,
    drvflash_BUSY,
    drvflash_DATA_OVERFLOW,
} drvflash_Status;


typedef struct{
    uint32_t address;
    uint32_t size;
    uint32_t write_ptr;
} drvflash_SectorInfo;

drvflash_Status drvflash_Init(void);
drvflash_Status drvflash_DeInit(void);
drvflash_Status drvflash_CheckHealth(void);
drvflash_Status drvflash_Unlock(uint8_t sector, uint32_t timeout);
drvflash_Status drvflash_Lock(uint8_t sector);
drvflash_Status drvflash_Erase(uint8_t sector, uint32_t timeout);
drvflash_Status drvflash_Write(uint8_t sector, uint8_t *data, uint32_t size,uint32_t timeout);
drvflash_Status drvflash_GetSectorInfo(uint8_t sector, drvflash_SectorInfo *info);
drvflash_Status drvflash_SetWritePtr(uint8_t sector, uint32_t address);


#endif // DRVFLASH_H