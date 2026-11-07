#include "driver/flash/drvflash.h"


static osMutexId_t drvflash_mutex_lock_task = NULL;
    
static uint8_t drvflash_flag_critical_sector[DRVFLASH_MAX_SECTORS]={
    1,  // Sector 0: 16 KB (Bootloader - Bất khả xâm phạm)
    0,  // Sector 1: 16 KB (Metadata Slot 1)
    0,  // Sector 2: 16 KB (Metadata Slot 2)
    0,  // Sector 3: 16 KB (Checkpoint)
    0,  // Sector 4: 64 KB
    0,  // Sector 5: 128 KB
    0,  // Sector 6: 128 KB
    0   // Sector 7: 128 KB
};

static osSemaphoreId_t drvflash_Sema[DRVFLASH_MAX_SECTORS] = {NULL};

static drvflash_SectorInfo drvflash_sector_info[DRVFLASH_MAX_SECTORS] = {
    {0x08000000, 16*1024, 0x08000000},
    {0x08004000, 16*1024, 0x08004000},
    {0x08008000, 16*1024, 0x08008000},
    {0x0800C000, 16*1024, 0x0800C000},
    {0x08010000, 64*1024, 0x08010000},
    {0x08020000, 128*1024, 0x08020000},
    {0x08040000, 128*1024, 0x08040000},
    {0x08060000, 128*1024, 0x08060000}
}; //đơn vị là bytes

drvflash_Status drvflash_Init(void)
{
    drvflash_Status status = drvflash_OK;
    drvflash_mutex_lock_task = osMutexNew(NULL);
    for(uint8_t i = 0; i < DRVFLASH_MAX_SECTORS; i++) {
        drvflash_Sema[i] = osSemaphoreNew(1, 1, NULL); // Lock all sectors by default
    }

    /* Kiểm tra VTOR / Reset Handler để xác định Slot đang chạy và khóa cặp phân vùng Active */
    uint32_t vtor = SCB->VTOR;
    uint32_t reset_handler = (vtor >= 0x20000000 && vtor < 0x20020000) ? 
                             *(volatile uint32_t *)(vtor + 4) : vtor;

    if (reset_handler >= 0x08020000 && reset_handler < 0x08040000) {
        /* Đang chạy Slot 1 -> Khóa cứng Sector 5 (App 1) và Sector 1 (Meta Slot 1) */
        drvflash_flag_critical_sector[5] = 1;
        drvflash_flag_critical_sector[1] = 1;
    } else if (reset_handler >= 0x08040000 && reset_handler < 0x08060000) {
        /* Đang chạy Slot 2 -> Khóa cứng Sector 6 (App 2) và Sector 2 (Meta Slot 2) */
        drvflash_flag_critical_sector[6] = 1;
        drvflash_flag_critical_sector[2] = 1;
    }

    status = drvflash_CheckHealth();
    if (status != drvflash_OK) drvflash_DeInit(); // Clean up if initialization failed
    return status;
}
drvflash_Status drvflash_DeInit(void)
{
    drvflash_Status status = drvflash_OK;
    if (drvflash_mutex_lock_task != NULL) {
        osStatus_t stat = osMutexDelete(drvflash_mutex_lock_task);
        drvflash_mutex_lock_task = NULL;
        if (stat != osOK) status = drvflash_ERROR;
    }
    for(int8_t i = 0; i < DRVFLASH_MAX_SECTORS; i++) {
        if (drvflash_Sema[i] != NULL) {
            osStatus_t stat = osSemaphoreDelete(drvflash_Sema[i]);
            drvflash_Sema[i] = NULL;
            if (stat != osOK) status = drvflash_ERROR;
        }
    }
    return status;
}
drvflash_Status drvflash_CheckHealth(void)
{
    drvflash_Status status = drvflash_OK;
    if( (drvflash_mutex_lock_task == NULL) ) status = drvflash_ERROR;
     for(uint8_t i = 0; i < DRVFLASH_MAX_SECTORS; i++) {
        if (drvflash_Sema[i] == NULL) {
            status = drvflash_ERROR;
            break;
        }
    }
    return status;
}
drvflash_Status drvflash_Unlock(uint8_t sector, uint32_t timeout){
    
    drvflash_Status status = drvflash_OK;

    if( (drvflash_CheckHealth() != drvflash_OK  ) ||
        (sector >= DRVFLASH_MAX_SECTORS         ) ||
        (drvflash_flag_critical_sector[sector]  ) ) return drvflash_ERROR; // Cannot unlock critical sectors

    if(osSemaphoreAcquire(drvflash_Sema[sector], timeout) != osOK) status = drvflash_BUSY;

    return status;
};

drvflash_Status drvflash_Lock(uint8_t sector){
    drvflash_Status status = drvflash_OK;

    if( (drvflash_CheckHealth() != drvflash_OK    ) ||
        (sector >= DRVFLASH_MAX_SECTORS           ) ||
        (drvflash_flag_critical_sector[sector]    ) ) return drvflash_ERROR;

    osSemaphoreRelease(drvflash_Sema[sector]);

    return status;
}
drvflash_Status drvflash_Erase(uint8_t sector,uint32_t timeout)
{
    if( (drvflash_CheckHealth() != drvflash_OK  ) || 
        (sector >= DRVFLASH_MAX_SECTORS         ) ) return drvflash_ERROR; // Sector đang bị khóa
    if(drvflash_flag_critical_sector[sector]) return drvflash_ERROR; // Không được xóa các sector quan trọng
    if (osSemaphoreGetCount(drvflash_Sema[sector]) != 0) return drvflash_ERROR; // Sector đang bị khóa
    if(osMutexAcquire(drvflash_mutex_lock_task, timeout) != osOK) return drvflash_BUSY;

    drvflash_SectorInfo *info = &drvflash_sector_info[sector];
  
    FLASH_EraseInitTypeDef eraseInitStruct;
    eraseInitStruct.TypeErase    = FLASH_TYPEERASE_SECTORS;
    eraseInitStruct.Sector       = sector;
    eraseInitStruct.NbSectors    = 1;
    eraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;

    uint32_t SectorError = 0;
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP   | 
                           FLASH_FLAG_OPERR | 
                           FLASH_FLAG_WRPERR|        
                           FLASH_FLAG_PGAERR| 
                           FLASH_FLAG_PGPERR| 
                           FLASH_FLAG_PGSERR);

    drvflash_Status status = drvflash_OK;

    if (HAL_FLASH_Unlock() == HAL_OK) {
        if (HAL_FLASHEx_Erase(&eraseInitStruct, &SectorError) != HAL_OK) {
            status = drvflash_ERROR;
        }
        HAL_FLASH_Lock();
        
        __HAL_FLASH_DATA_CACHE_DISABLE();
        __HAL_FLASH_DATA_CACHE_RESET();
        __HAL_FLASH_DATA_CACHE_ENABLE();
    } else status = drvflash_ERROR;
    if (status == drvflash_OK) info->write_ptr = info->address; // Reset the current address after erase
    osMutexRelease(drvflash_mutex_lock_task);
    return status;
}

drvflash_Status drvflash_Write(uint8_t sector, uint8_t *data, uint32_t size,uint32_t timeout)
{
    drvflash_Status status = drvflash_OK;

    if( (drvflash_CheckHealth() != drvflash_OK                          ) ||
        (sector >= DRVFLASH_MAX_SECTORS || data == NULL || size == 0 ) ) return drvflash_ERROR; // Sector is locked
    if(drvflash_flag_critical_sector[sector]) return drvflash_ERROR;
    drvflash_SectorInfo *info = &drvflash_sector_info[sector];
    if (info->write_ptr - info->address + size > info->size) return drvflash_DATA_OVERFLOW; // 
    if (osSemaphoreGetCount(drvflash_Sema[sector]) != 0) return drvflash_ERROR;
    if(osMutexAcquire(drvflash_mutex_lock_task, timeout) != osOK) return drvflash_ERROR;

    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP   | 
                           FLASH_FLAG_OPERR | 
                           FLASH_FLAG_WRPERR|        
                           FLASH_FLAG_PGAERR| 
                           FLASH_FLAG_PGPERR| 
                           FLASH_FLAG_PGSERR);
    
    
    if (HAL_FLASH_Unlock() == HAL_OK) 
    {
        for (uint32_t i = 0; i < size; ) {
            if ((size - i) >= 4) {
                uint32_t temp = (uint32_t)data[i] | 
                               ((uint32_t)data[i + 1] << 8) | 
                               ((uint32_t)data[i + 2] << 16) | 
                               ((uint32_t)data[i + 3] << 24);

                if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, info->write_ptr + i, (uint64_t)temp) != HAL_OK) {
                    status = drvflash_ERROR;
                    break;
                }
                i += 4;
            } else {
                if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, info->write_ptr + i, (uint64_t)data[i]) != HAL_OK) {
                    status = drvflash_ERROR;
                    break;
                }
                i++;
            }
        }
        HAL_FLASH_Lock();
        
   
        __HAL_FLASH_DATA_CACHE_DISABLE();
        __HAL_FLASH_DATA_CACHE_RESET();
        __HAL_FLASH_DATA_CACHE_ENABLE();
    } 
    else status = drvflash_ERROR;

    if (status == drvflash_OK) info ->write_ptr += size;
    osMutexRelease(drvflash_mutex_lock_task);

    return status;
}

drvflash_Status drvflash_GetSectorInfo(uint8_t sector, drvflash_SectorInfo *info)
{
     if( (drvflash_CheckHealth() != drvflash_OK ) || 
        (sector >= DRVFLASH_MAX_SECTORS         ) ||
        (info == NULL                           ) ) return drvflash_ERROR;
    *info = drvflash_sector_info[sector];
    return drvflash_OK;
}

drvflash_Status drvflash_SetWritePtr(uint8_t sector, uint32_t address){
    
    drvflash_Status status = drvflash_OK;
    
    if( (drvflash_CheckHealth() != drvflash_OK ) || 
        (sector >= DRVFLASH_MAX_SECTORS         ) ||
        (address < drvflash_sector_info[sector].address) ||
        (address > drvflash_sector_info[sector].address + drvflash_sector_info[sector].size) ) return drvflash_ERROR;
    if (osSemaphoreGetCount(drvflash_Sema[sector]) != 0) return drvflash_ERROR;
    if (address > drvflash_sector_info[sector].address + drvflash_sector_info[sector].size) return drvflash_ERROR;
    drvflash_sector_info[sector].write_ptr = address;
    return status;
}