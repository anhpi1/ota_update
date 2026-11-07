/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "hook/apphook.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t hook_defaultTaskHandle;
const osThreadAttr_t hook_defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void __wrap_StartDefaultTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void __wrap_MX_FREERTOS_Init(void) {
  void (*app_fn)(void) = (void (*)(void))APP_ROUTER_TABLE[APP_VEC_FREERTOS_INIT];
  if (app_fn != 0)
  {
    app_fn();
    return;
  }

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  hook_defaultTaskHandle = osThreadNew(__wrap_StartDefaultTask, NULL, &hook_defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void __wrap_StartDefaultTask(void *argument)
{
  void (*app_fn)(void *) = (void (*)(void *))APP_ROUTER_TABLE[APP_VEC_DEFAULT_TASK];
  if (app_fn != 0)
  {
    app_fn(argument);
    return;
  }

  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void __wrap_vApplicationIdleHook(void)
{
  void (*app_fn)(void) = (void (*)(void))APP_ROUTER_TABLE[APP_VEC_RTOS_IDLE_HOOK];
  if (app_fn != 0) app_fn();
}

void __wrap_vApplicationTickHook(void)
{
  void (*app_fn)(void) = (void (*)(void))APP_ROUTER_TABLE[APP_VEC_RTOS_TICK_HOOK];
  if (app_fn != 0) app_fn();
}

void __wrap_vApplicationStackOverflowHook(TaskHandle_t xTask, signed char *pcTaskName)
{
  void (*app_fn)(TaskHandle_t, signed char *) = (void (*)(TaskHandle_t, signed char *))APP_ROUTER_TABLE[APP_VEC_RTOS_STACK_HOOK];
  if (app_fn != 0) app_fn(xTask, pcTaskName);
}

void __wrap_vApplicationMallocFailedHook(void)
{
  void (*app_fn)(void) = (void (*)(void))APP_ROUTER_TABLE[APP_VEC_RTOS_MALLOC_FAIL];
  if (app_fn != 0) app_fn();
}

void __wrap_vApplicationDaemonTaskStartupHook(void)
{
  void (*app_fn)(void) = (void (*)(void))APP_ROUTER_TABLE[APP_VEC_RTOS_DAEMON_HOOK];
  if (app_fn != 0) app_fn();
}
/* USER CODE END Application */

