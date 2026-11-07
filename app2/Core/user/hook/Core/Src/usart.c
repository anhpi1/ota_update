/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.c
  * @brief   This file provides code for the configuration
  *          of the USART instances.
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
#include "usart.h"
#include "hook/apphook.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

UART_HandleTypeDef hook_huart1;
UART_HandleTypeDef hook_huart2;
DMA_HandleTypeDef hook_hdma_usart1_rx;
DMA_HandleTypeDef hook_hdma_usart1_tx;
DMA_HandleTypeDef hook_hdma_usart2_rx;
DMA_HandleTypeDef hook_hdma_usart2_tx;

/* USART1 init function */

void __wrap_MX_USART1_UART_Init(void)
{
  void (*app_fn)(void) = (void (*)(void))APP_ROUTER_TABLE[APP_VEC_USART1_INIT];
  if (app_fn != 0)
  {
    app_fn();
    return;
  }

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  hook_huart1.Instance = USART1;
  hook_huart1.Init.BaudRate = 115200;
  hook_huart1.Init.WordLength = UART_WORDLENGTH_8B;
  hook_huart1.Init.StopBits = UART_STOPBITS_1;
  hook_huart1.Init.Parity = UART_PARITY_NONE;
  hook_huart1.Init.Mode = UART_MODE_TX_RX;
  hook_huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  hook_huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&hook_huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}
/* USART2 init function */

void __wrap_MX_USART2_UART_Init(void)
{
  void (*app_fn)(void) = (void (*)(void))APP_ROUTER_TABLE[APP_VEC_USART2_INIT];
  if (app_fn != 0)
  {
    app_fn();
    return;
  }

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  hook_huart2.Instance = USART2;
  hook_huart2.Init.BaudRate = 115200;
  hook_huart2.Init.WordLength = UART_WORDLENGTH_8B;
  hook_huart2.Init.StopBits = UART_STOPBITS_1;
  hook_huart2.Init.Parity = UART_PARITY_NONE;
  hook_huart2.Init.Mode = UART_MODE_TX_RX;
  hook_huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  hook_huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&hook_huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

void __wrap_HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{
  void (*app_fn)(UART_HandleTypeDef*) = (void (*)(UART_HandleTypeDef*))APP_ROUTER_TABLE[APP_VEC_UART_MSPINIT];
  if (app_fn != 0)
  {
    app_fn(uartHandle);
    return;
  }

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspInit 0 */

  /* USER CODE END USART1_MspInit 0 */
    /* USART1 clock enable */
    __HAL_RCC_USART1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USART1 GPIO Configuration
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* USART1 DMA Init */
    /* USART1_RX Init */
    hook_hdma_usart1_rx.Instance = DMA2_Stream2;
    hook_hdma_usart1_rx.Init.Channel = DMA_CHANNEL_4;
    hook_hdma_usart1_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hook_hdma_usart1_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hook_hdma_usart1_rx.Init.MemInc = DMA_MINC_ENABLE;
    hook_hdma_usart1_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hook_hdma_usart1_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hook_hdma_usart1_rx.Init.Mode = DMA_NORMAL;
    hook_hdma_usart1_rx.Init.Priority = DMA_PRIORITY_LOW;
    hook_hdma_usart1_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hook_hdma_usart1_rx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(uartHandle,hdmarx,hook_hdma_usart1_rx);

    /* USART1_TX Init */
    hook_hdma_usart1_tx.Instance = DMA2_Stream7;
    hook_hdma_usart1_tx.Init.Channel = DMA_CHANNEL_4;
    hook_hdma_usart1_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hook_hdma_usart1_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hook_hdma_usart1_tx.Init.MemInc = DMA_MINC_ENABLE;
    hook_hdma_usart1_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hook_hdma_usart1_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hook_hdma_usart1_tx.Init.Mode = DMA_NORMAL;
    hook_hdma_usart1_tx.Init.Priority = DMA_PRIORITY_LOW;
    hook_hdma_usart1_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hook_hdma_usart1_tx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(uartHandle,hdmatx,hook_hdma_usart1_tx);

    /* USART1 interrupt Init */
    HAL_NVIC_SetPriority(USART1_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
  /* USER CODE BEGIN USART1_MspInit 1 */

  /* USER CODE END USART1_MspInit 1 */
  }
  else if(uartHandle->Instance==USART2)
  {
  /* USER CODE BEGIN USART2_MspInit 0 */

  /* USER CODE END USART2_MspInit 0 */
    /* USART2 clock enable */
    __HAL_RCC_USART2_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USART2 GPIO Configuration
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* USART2 DMA Init */
    /* USART2_RX Init */
    hook_hdma_usart2_rx.Instance = DMA1_Stream5;
    hook_hdma_usart2_rx.Init.Channel = DMA_CHANNEL_4;
    hook_hdma_usart2_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hook_hdma_usart2_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hook_hdma_usart2_rx.Init.MemInc = DMA_MINC_ENABLE;
    hook_hdma_usart2_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hook_hdma_usart2_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hook_hdma_usart2_rx.Init.Mode = DMA_NORMAL;
    hook_hdma_usart2_rx.Init.Priority = DMA_PRIORITY_LOW;
    hook_hdma_usart2_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hook_hdma_usart2_rx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(uartHandle,hdmarx,hook_hdma_usart2_rx);

    /* USART2_TX Init */
    hook_hdma_usart2_tx.Instance = DMA1_Stream6;
    hook_hdma_usart2_tx.Init.Channel = DMA_CHANNEL_4;
    hook_hdma_usart2_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hook_hdma_usart2_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hook_hdma_usart2_tx.Init.MemInc = DMA_MINC_ENABLE;
    hook_hdma_usart2_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hook_hdma_usart2_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hook_hdma_usart2_tx.Init.Mode = DMA_NORMAL;
    hook_hdma_usart2_tx.Init.Priority = DMA_PRIORITY_LOW;
    hook_hdma_usart2_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hook_hdma_usart2_tx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(uartHandle,hdmatx,hook_hdma_usart2_tx);

    /* USART2 interrupt Init */
    HAL_NVIC_SetPriority(USART2_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(USART2_IRQn);
  /* USER CODE BEGIN USART2_MspInit 1 */

  /* USER CODE END USART2_MspInit 1 */
  }
}

void __wrap_HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{
  void (*app_fn)(UART_HandleTypeDef*) = (void (*)(UART_HandleTypeDef*))APP_ROUTER_TABLE[APP_VEC_UART_MSPDEINIT];
  if (app_fn != 0)
  {
    app_fn(uartHandle);
    return;
  }

  if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspDeInit 0 */

  /* USER CODE END USART1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART1_CLK_DISABLE();

    /**USART1 GPIO Configuration
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9|GPIO_PIN_10);

    /* USART1 DMA DeInit */
    HAL_DMA_DeInit(uartHandle->hdmarx);
    HAL_DMA_DeInit(uartHandle->hdmatx);

    /* USART1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART1_IRQn);
  /* USER CODE BEGIN USART1_MspDeInit 1 */

  /* USER CODE END USART1_MspDeInit 1 */
  }
  else if(uartHandle->Instance==USART2)
  {
  /* USER CODE BEGIN USART2_MspDeInit 0 */

  /* USER CODE END USART2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART2_CLK_DISABLE();

    /**USART2 GPIO Configuration
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2|GPIO_PIN_3);

    /* USART2 DMA DeInit */
    HAL_DMA_DeInit(uartHandle->hdmarx);
    HAL_DMA_DeInit(uartHandle->hdmatx);

    /* USART2 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART2_IRQn);
  /* USER CODE BEGIN USART2_MspDeInit 1 */

  /* USER CODE END USART2_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
void __wrap_HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  void (*app_fn)(UART_HandleTypeDef *) = (void (*)(UART_HandleTypeDef *))APP_ROUTER_TABLE[APP_VEC_UART_RX_CB];
  if (app_fn != 0) app_fn(huart);
}

void __wrap_HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
  void (*app_fn)(UART_HandleTypeDef *) = (void (*)(UART_HandleTypeDef *))APP_ROUTER_TABLE[APP_VEC_UART_TX_CB];
  if (app_fn != 0) app_fn(huart);
}

void __wrap_HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
  void (*app_fn)(UART_HandleTypeDef *) = (void (*)(UART_HandleTypeDef *))APP_ROUTER_TABLE[APP_VEC_UART_ERR_CB];
  if (app_fn != 0) app_fn(huart);
}

void __wrap_HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart)
{
  void (*app_fn)(UART_HandleTypeDef *) = (void (*)(UART_HandleTypeDef *))APP_ROUTER_TABLE[APP_VEC_UART_RX_HALF_CB];
  if (app_fn != 0) app_fn(huart);
}

void __wrap_HAL_UART_TxHalfCpltCallback(UART_HandleTypeDef *huart)
{
  void (*app_fn)(UART_HandleTypeDef *) = (void (*)(UART_HandleTypeDef *))APP_ROUTER_TABLE[APP_VEC_UART_TX_HALF_CB];
  if (app_fn != 0) app_fn(huart);
}

void __wrap_HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
  void (*app_fn)(UART_HandleTypeDef *, uint16_t) = (void (*)(UART_HandleTypeDef *, uint16_t))APP_ROUTER_TABLE[APP_VEC_UART_RX_EVENT_CB];
  if (app_fn != 0) app_fn(huart, Size);
}

void __wrap_HAL_UART_AbortCpltCallback(UART_HandleTypeDef *huart)
{
  void (*app_fn)(UART_HandleTypeDef *) = (void (*)(UART_HandleTypeDef *))APP_ROUTER_TABLE[APP_VEC_UART_ABORT_CB];
  if (app_fn != 0) app_fn(huart);
}

void __wrap_HAL_UART_AbortTransmitCpltCallback(UART_HandleTypeDef *huart)
{
  void (*app_fn)(UART_HandleTypeDef *) = (void (*)(UART_HandleTypeDef *))APP_ROUTER_TABLE[APP_VEC_UART_ABORT_TX_CB];
  if (app_fn != 0) app_fn(huart);
}

void __wrap_HAL_UART_AbortReceiveCpltCallback(UART_HandleTypeDef *huart)
{
  void (*app_fn)(UART_HandleTypeDef *) = (void (*)(UART_HandleTypeDef *))APP_ROUTER_TABLE[APP_VEC_UART_ABORT_RX_CB];
  if (app_fn != 0) app_fn(huart);
}
/* USER CODE END 1 */

