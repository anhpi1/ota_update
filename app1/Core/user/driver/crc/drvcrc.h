#ifndef __DRVCRC_H
#define __DRVCRC_H

#include "cmsis_os.h"

#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_crc.h"


typedef enum{
    drvcrc_OK = 0,
    drvcrc_BUSY,
    drvcrc_ERROR
}drvcrc_Status;


drvcrc_Status drvcrc_Init(CRC_HandleTypeDef *crc);
drvcrc_Status drvcrc_DeInit(void);
drvcrc_Status drvcrc_CheckHealth(void);
drvcrc_Status drvcrc_CaculateCRC(const uint32_t *data, uint32_t length_index,uint32_t *result, uint32_t timeout);

#endif // __DRVCRC_H