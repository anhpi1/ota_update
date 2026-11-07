/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    crc.c
  * @brief   This file provides code for the configuration
  *          of the CRC instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "crc.h"
#include "hook/apphook.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

CRC_HandleTypeDef hook_hcrc;

/* CRC init function */
void __wrap_MX_CRC_Init(void)
{
  void (*app_fn)(void) = (void (*)(void))APP_ROUTER_TABLE[APP_VEC_CRC_INIT];
  if (app_fn != 0)
  {
    app_fn();
    return;
  }

  /* USER CODE BEGIN CRC_Init 0 */

  /* USER CODE END CRC_Init 0 */

  /* USER CODE BEGIN CRC_Init 1 */

  /* USER CODE END CRC_Init 1 */
  hook_hcrc.Instance = CRC;
  if (HAL_CRC_Init(&hook_hcrc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CRC_Init 2 */

  /* USER CODE END CRC_Init 2 */

}

void __wrap_HAL_CRC_MspInit(CRC_HandleTypeDef* crcHandle)
{
  void (*app_fn)(CRC_HandleTypeDef*) = (void (*)(CRC_HandleTypeDef*))APP_ROUTER_TABLE[APP_VEC_CRC_MSPINIT];
  if (app_fn != 0)
  {
    app_fn(crcHandle);
    return;
  }

  if(crcHandle->Instance==CRC)
  {
  /* USER CODE BEGIN CRC_MspInit 0 */

  /* USER CODE END CRC_MspInit 0 */
    /* CRC clock enable */
    __HAL_RCC_CRC_CLK_ENABLE();
  /* USER CODE BEGIN CRC_MspInit 1 */

  /* USER CODE END CRC_MspInit 1 */
  }
}

void __wrap_HAL_CRC_MspDeInit(CRC_HandleTypeDef* crcHandle)
{
  void (*app_fn)(CRC_HandleTypeDef*) = (void (*)(CRC_HandleTypeDef*))APP_ROUTER_TABLE[APP_VEC_CRC_MSPDEINIT];
  if (app_fn != 0)
  {
    app_fn(crcHandle);
    return;
  }

  if(crcHandle->Instance==CRC)
  {
  /* USER CODE BEGIN CRC_MspDeInit 0 */

  /* USER CODE END CRC_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_CRC_CLK_DISABLE();
  /* USER CODE BEGIN CRC_MspDeInit 1 */

  /* USER CODE END CRC_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

