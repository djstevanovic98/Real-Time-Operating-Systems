/*
 * vezbanje.c
 *
 *  Created on: Oct 13, 2020
 *      Author: dzou
 */

// upisujemo 3 promenjive 0, 1000, 2000 i citamo ih svaki put, povecavamo za 1

/* Standard includes. */
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#define QUEUE_SIZE 10	// red velicine 10

static int xMyQueue[QUEUE_SIZE];	// x za nizovi i strukture
static int iWritePtr = 0;	//write i read iteracija
static int iReadPtr = 0;

static int IsQueueEmpty(){
	return iWritePtr == iReadPtr;
}

static int IsQueueFull(){
	return ((iWritePtr+1) % QUEUE_SIZE) == iReadPtr;
}

static xSemaphoreHandle xWriteMutex;
static xSemaphoreHandle xQueueFull;
static xSemaphoreHandle xQueueEmpty;

static void vQueuePut(int val){

	if(xSemaphoreTake(xQueueFull,(TickType_t) portMAX_DELAY) == pdTRUE){

		if(xSemaphoreTake(xWriteMutex,(TickType_t) portMAX_DELAY) == pdTRUE){

			xMyQueue[iWritePtr] = val;
			iWritePtr = (iWritePtr + 1) % QUEUE_SIZE;

			xSemaphoreGive(xQueueEmpty);

			xSemaphoreGive(xWriteMutex);
		}
	}
}

static int iQueueTake(){
	int val = -1;

	if(xSemaphoreTake(xQueueEmpty, (TickType_t) portMAX_DELAY) == pdTRUE){
		val = xMyQueue[iReadPtr];
		iReadPtr = (iReadPtr + 1) % QUEUE_SIZE;

		xSemaphoreGive(xQueueFull);
	}

	return val;
}

static void vWriter(void* pvParametri){
	int* val = (int*) pvParametri;
	int i;

	for(i = *val; i<(*val)+100;){
		if(!IsQueueFull()){
			vQueuePut(i);

			printf("Writen val is: %d\n", i);
			fflush(stdout);

			vTaskDelay((TickType_t) 500);
			i++;
		}
	}

	vTaskDelete(0);
}

static void vReader(void* pvParametri){

	for(;;)
	{

	if(!IsQueueEmpty()){
		int val = iQueueTake();

		printf("Read val is: %d\n", val);
		fflush(stdout);

		vTaskDelay((TickType_t) 100);
		}
	}
	vTaskDelete(0);
}


int vezbanje( void ){

	int val1 = 0;
	int val2 = 1000;
	int val3 = 2000;

	xTaskCreate(vWriter, "ispis", configMINIMAL_STACK_SIZE, &val1, 1, NULL);
	xTaskCreate(vWriter, "ispis", configMINIMAL_STACK_SIZE, &val2, 1, NULL);
	xTaskCreate(vWriter, "ispis", configMINIMAL_STACK_SIZE, &val3, 1, NULL);

	xTaskCreate(vReader, "read", configMINIMAL_STACK_SIZE, NULL, 1, NULL);

	xWriteMutex = xSemaphoreCreateMutex();
	xQueueFull = xSemaphoreCreateCounting(QUEUE_SIZE, QUEUE_SIZE-1);
	xQueueEmpty = xSemaphoreCreateCounting(QUEUE_SIZE, 0);

	vTaskStartScheduler();


	return 0;
}




