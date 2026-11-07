/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file         stm32f4xx_hal_msp.c
  * @brief        This file provides code for the MSP Initialization
  *               and de-Initialization codes.
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
#include "main.h"
#include "hook/apphook.h"
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN Define */

/* USER CODE END Define */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN Macro */

/* USER CODE END Macro */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* External functions --------------------------------------------------------*/
/* USER CODE BEGIN ExternalFunctions */

/* USER CODE END ExternalFunctions */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */
/**
  * Initializes the Global MSP.
  */
void __wrap_HAL_MspInit(void)
{
  void (*app_fn)(void) = (void (*)(void))APP_ROUTER_TABLE[APP_VEC_HAL_MSPINIT];
  if (app_fn != 0)
  {
    app_fn();
    return;
  }

  /* USER CODE BEGIN MspInit 0 */

  /* USER CODE END MspInit 0 */

  __HAL_RCC_SYSCFG_CLK_ENABLE();
  __HAL_RCC_PWR_CLK_ENABLE();

  /* System interrupt init*/
  /* PendSV_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(PendSV_IRQn, 15, 0);

  /* USER CODE BEGIN MspInit 1 */

  /* USER CODE END MspInit 1 */
}

/* USER CODE BEGIN 1 */
void __wrap_HAL_SYSTICK_Callback(void)
{
  void (*app_fn)(void) = (void (*)(void))APP_ROUTER_TABLE[APP_VEC_SYSTICK_CB];
  if (app_fn != 0) app_fn();
}

void __wrap_HAL_PWR_PVDCallback(void)
{
  void (*app_fn)(void) = (void (*)(void))APP_ROUTER_TABLE[APP_VEC_PWR_PVD_CB];
  if (app_fn != 0) app_fn();
}

void __wrap_HAL_RCC_CSSCallback(void)
{
  void (*app_fn)(void) = (void (*)(void))APP_ROUTER_TABLE[APP_VEC_RCC_CSS_CB];
  if (app_fn != 0) app_fn();
}

void __wrap_HAL_FLASH_EndOfOperationCallback(uint32_t ReturnValue)
{
  void (*app_fn)(uint32_t) = (void (*)(uint32_t))APP_ROUTER_TABLE[APP_VEC_FLASH_END_CB];
  if (app_fn != 0) app_fn(ReturnValue);
}

void __wrap_HAL_FLASH_OperationErrorCallback(uint32_t ReturnValue)
{
  void (*app_fn)(uint32_t) = (void (*)(uint32_t))APP_ROUTER_TABLE[APP_VEC_FLASH_ERR_CB];
  if (app_fn != 0) app_fn(ReturnValue);
}
/* USER CODE END 1 */
