/*
 * tasks_robocom.h
 *
 *  Created on: 25 janv. 2022
 *      Author: bosti
 */

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "stm32f4xx_hal.h"

#ifndef INC_TASKS_ROBOCOM_H_
#define INC_TASKS_ROBOCOM_H_

extern SemaphoreHandle_t xMutex;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;


xTaskHandle SendertaskHC12_handler;
xTaskHandle ReceivertaskHC12_handler;
xTaskHandle SendertaskNRF_handler;
xTaskHandle ReceivertaskNRF_handler;

xQueueHandle simpleQueue;
xQueueHandle simpleQueue2;
xQueueHandle simpleQueue3;
xQueueHandle simpleQueue4;

void taskUartPc(void* argument);
void taskUartHc12(void* argument);
void taskNrfRx(void* argument);
void taskNrfTx(void* argument);

#endif /* INC_TASKS_ROBOCOM_H_ */
