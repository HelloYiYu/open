/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : app_freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
osThreadId default_threadHandle;
osThreadId main_threadHandle;
osThreadId lp262_threadHandle;
osThreadId uart_threadHandle;
osMutexId fsm_event_mtxHandle;
osMutexId uart1_mtxHandle;
osMutexId uart2_mtxHandle;
osMutexId uart3_mtxHandle;
osMutexId uart4_mtxHandle;
osSemaphoreId uart1_send_semHandle;
osSemaphoreId uart2_send_semHandle;
osSemaphoreId uart3_send_semHandle;
osSemaphoreId uart4_send_semHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
extern void lp262_thread_init(void);
extern void uart_thread_init(void);
extern void main_thread_init(void);
extern void debug_init(void);
/* USER CODE END FunctionPrototypes */

void default_thread_func(void const * argument);
extern void main_thread_func(void const * argument);
extern void lp262_thread_func(void const * argument);
extern void uart_thread_func(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* Hook prototypes */
void configureTimerForRunTimeStats(void);
unsigned long getRunTimeCounterValue(void);
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName);

/* USER CODE BEGIN 1 */
/* Functions needed when configGENERATE_RUN_TIME_STATS is on */
__weak void configureTimerForRunTimeStats(void)
{

}

__weak unsigned long getRunTimeCounterValue(void)
{
return 0;
}
/* USER CODE END 1 */

/* USER CODE BEGIN 4 */
__weak void vApplicationStackOverflowHook(TaskHandle_t xTask, signed char *pcTaskName)
{
   /* Run time stack overflow checking is performed if
   configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2. This hook function is
   called if a stack overflow is detected. */
}
/* USER CODE END 4 */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
  lp262_thread_init();
  uart_thread_init();
  main_thread_init();
  // keep debug init at the last
  debug_init();
  /* USER CODE END Init */
  /* Create the mutex(es) */
  /* definition and creation of fsm_event_mtx */
  osMutexDef(fsm_event_mtx);
  fsm_event_mtxHandle = osMutexCreate(osMutex(fsm_event_mtx));

  /* definition and creation of uart1_mtx */
  osMutexDef(uart1_mtx);
  uart1_mtxHandle = osMutexCreate(osMutex(uart1_mtx));

  /* definition and creation of uart2_mtx */
  osMutexDef(uart2_mtx);
  uart2_mtxHandle = osMutexCreate(osMutex(uart2_mtx));

  /* definition and creation of uart3_mtx */
  osMutexDef(uart3_mtx);
  uart3_mtxHandle = osMutexCreate(osMutex(uart3_mtx));

  /* definition and creation of uart4_mtx */
  osMutexDef(uart4_mtx);
  uart4_mtxHandle = osMutexCreate(osMutex(uart4_mtx));

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* definition and creation of uart1_send_sem */
  osSemaphoreDef(uart1_send_sem);
  uart1_send_semHandle = osSemaphoreCreate(osSemaphore(uart1_send_sem), 1);

  /* definition and creation of uart2_send_sem */
  osSemaphoreDef(uart2_send_sem);
  uart2_send_semHandle = osSemaphoreCreate(osSemaphore(uart2_send_sem), 1);

  /* definition and creation of uart3_send_sem */
  osSemaphoreDef(uart3_send_sem);
  uart3_send_semHandle = osSemaphoreCreate(osSemaphore(uart3_send_sem), 1);

  /* definition and creation of uart4_send_sem */
  osSemaphoreDef(uart4_send_sem);
  uart4_send_semHandle = osSemaphoreCreate(osSemaphore(uart4_send_sem), 1);

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
  /* definition and creation of default_thread */
  osThreadDef(default_thread, default_thread_func, osPriorityIdle, 0, 128);
  default_threadHandle = osThreadCreate(osThread(default_thread), NULL);

  /* definition and creation of main_thread */
  osThreadDef(main_thread, main_thread_func, osPriorityNormal, 0, 128);
  main_threadHandle = osThreadCreate(osThread(main_thread), NULL);

  /* definition and creation of lp262_thread */
  osThreadDef(lp262_thread, lp262_thread_func, osPriorityNormal, 0, 128);
  lp262_threadHandle = osThreadCreate(osThread(lp262_thread), NULL);

  /* definition and creation of uart_thread */
  osThreadDef(uart_thread, uart_thread_func, osPriorityRealtime, 0, 128);
  uart_threadHandle = osThreadCreate(osThread(uart_thread), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_default_thread_func */
/**
  * @brief  Function implementing the default_thread thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_default_thread_func */
void default_thread_func(void const * argument)
{
  /* USER CODE BEGIN default_thread_func */
  /* Infinite loop */
  for(;;)
  {
	HAL_GPIO_TogglePin(LED4_GPIO_Port, LED4_Pin);
    osDelay(500);
  }
  /* USER CODE END default_thread_func */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

