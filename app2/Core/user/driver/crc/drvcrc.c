#include "drvcrc.h"


static CRC_HandleTypeDef *drvcrc_crc = NULL;

static osMutexId_t drvcrc_Mutex = NULL;

drvcrc_Status drvcrc_Init(CRC_HandleTypeDef *crc)
{
    drvcrc_Status status = drvcrc_OK;
    if(crc == NULL) return drvcrc_ERROR;
    if (HAL_CRC_GetState(crc) != HAL_CRC_STATE_READY) return drvcrc_ERROR;

    drvcrc_crc = crc;
    drvcrc_Mutex = osMutexNew(NULL);

    if (drvcrc_CheckHealth() != drvcrc_OK) status = drvcrc_ERROR;
    if(status != drvcrc_OK) drvcrc_DeInit();

    return status;
}
drvcrc_Status drvcrc_DeInit(void)
{
    drvcrc_Status status = drvcrc_OK;

    if(drvcrc_crc != NULL) drvcrc_crc = NULL;

    if (drvcrc_Mutex != NULL) {
        osStatus_t stat = osMutexDelete(drvcrc_Mutex);
        drvcrc_Mutex = NULL;
        if (stat != osOK) status = drvcrc_ERROR;
    }
    return status;
}

drvcrc_Status drvcrc_CheckHealth(void)
{
    drvcrc_Status status = drvcrc_OK;
    if( (drvcrc_crc == NULL)    ||
        (drvcrc_Mutex == NULL)  ) status = drvcrc_ERROR;
    if (HAL_CRC_GetState(drvcrc_crc) == HAL_CRC_STATE_RESET) return drvcrc_ERROR;


    
    return status;
}

drvcrc_Status drvcrc_CaculateCRC(const uint32_t *data, uint32_t length_index,uint32_t *result, uint32_t timeout)
{
    drvcrc_Status status = drvcrc_OK;

    if( (data == NULL || result == NULL)    ||
        (length_index == 0)                 ) return drvcrc_ERROR;
    if(drvcrc_CheckHealth() != drvcrc_OK) return drvcrc_ERROR;
    if(osMutexAcquire(drvcrc_Mutex, timeout) != osOK) return drvcrc_BUSY;


    *result = HAL_CRC_Calculate(drvcrc_crc, (uint32_t *)data, length_index);
    osMutexRelease(drvcrc_Mutex);

    return status;
}