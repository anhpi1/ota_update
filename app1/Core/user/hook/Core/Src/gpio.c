/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   This file provides code for the configuration
  *          of all used GPIO pins.
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
#include "gpio.h"
#include "hook/apphook.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/** Configure pins as
        * Analog
        * Input
        * Output
        * EVENT_OUT
        * EXTI
*/
void __wrap_MX_GPIO_Init(void)
{
  void (*app_fn)(void) = (void (*)(void))APP_ROUTER_TABLE[APP_VEC_GPIO_INIT];
  if (app_fn != 0)
  {
    app_fn();
    return;
  }

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

}

/* USER CODE BEGIN 2 */
void __wrap_HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  void (*app_fn)(uint16_t) = (void (*)(uint16_t))APP_ROUTER_TABLE[APP_VEC_GPIO_EXTI_CB];
  if (app_fn != 0)
  {
    app_fn(GPIO_Pin);
    return;
  }
}
/* USER CODE END 2 */
