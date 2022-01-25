/*
 * tasks_robocom.c
 *
 *  Created on: 25 janv. 2022
 *      Author: bosti
 */

#include "tasks_robocom.h"

SemaphoreHandle_t xMutex;



void Nrf_rx_init(uint64_t RxpipeAddrs)
{
	//NRF24_openWritingPipe(0);
	NRF24_flush_tx();
	NRF24_flush_rx();
	NRF24_openReadingPipe(1, RxpipeAddrs);
	NRF24_startListening();
}

void Nrf_tx_init(uint64_t TxpipeAddrs)
{
	NRF24_flush_tx();
	NRF24_flush_rx();
	NRF24_stopListening();
	NRF24_openWritingPipe(TxpipeAddrs);
	NRF24_openReadingPipe(1, 0);
}

void taskNrfRx(void * argument)
{
	uint32_t *myRxData = (char *) malloc( 32 ); // buffer pour contenir les data du nrf
	uint32_t *myRxData2 = (char *) malloc( 32 );
	uint32_t Tickdelay = pdMS_TO_TICKS(500);
	uint32_t val_crc = 0;
	uint32_t val_crc_verif = 0;
	char *b= (char *) malloc( 40 );//debug
	char *c= (char *) malloc( 40 );//debug
	while(1)
	{
		xSemaphoreTake( xMutex, portMAX_DELAY );
		Nrf_rx_init(0x11223344AA);
		HAL_Delay(500);
		if(NRF24_available())
		{
			NRF24_read(myRxData, 32);  // Réception du nrf
			strcpy(myRxData2,myRxData);
			val_crc = extractCrc(myRxData2); // Extraction du crc
			extractData(myRxData); //extraction des données (myRxData est modifié)
			//val_crc_verif = Tx_Adding_CRC((uint32_t*)myRxData, strlen(myRxData)); // calcul du CRC
			sprintf(b,"%lu",val_crc); // debug
			sprintf(c,"%lu",val_crc_verif); //debug

			if(val_crc == val_crc_verif) // Vérification du crc
			{
				//HAL_UART_Transmit(&huart2, (uint8_t *)"CRC OK\r\n", strlen("CRC OK\r\n"), 10);
			}
			else
			{
				//HAL_UART_Transmit(&huart2, (uint8_t *)"CRC False\r\n", strlen("CRC False\r\n"), 10);
			}
			myRxData[32] = '\r';
			myRxData[32+1] = '\n';
			xQueueSend(simpleQueue3,(void*)myRxData,34); // envoie vers uart
			//HAL_UART_Transmit(&huart2, (uint8_t *)b, strlen(b), 10);
			free(b);
			free(c);
			free(myRxData);
		}
		xSemaphoreGive( xMutex );
		vTaskDelay(Tickdelay);
	}
}

void taskNrfTx(void * argument)
{
	uint32_t Tickdelay = pdMS_TO_TICKS(500);
	char buffTxaa[32] = "";
	while(1)
	{
		xSemaphoreTake( xMutex, portMAX_DELAY );
		Nrf_tx_init(0x11223344AA);
		if (xQueueReceive(simpleQueue4, (void*)buffTxaa, 32) == pdTRUE) // réception depuis l'uart
		{
		}
		if(strlen(buffTxaa)>0){
			Tx_Adding_CRC((uint8_t *)buffTxaa,strlen(buffTxaa)); // calcul du crc
			if(NRF24_write(buffTxaa, 32)) // envoie vers le nrf
			{
				HAL_UART_Transmit(&huart2, (uint8_t *)buffTxaa, strlen(buffTxaa), 20);
			}
			strcpy(buffTxaa,"\0");
		}
		xSemaphoreGive( xMutex );
		vTaskDelay(Tickdelay);
	}
}

void taskUartPc(void* argument)
{
	Ringbuf_init();
	uint32_t Tickdelay = pdMS_TO_TICKS(500);
	char buff[32];
	char buffTx[32];
	char buff_Rx[32];
	char buff_Rx2[32];
	while(1)
	{
		//fill the buffer
		while(IsDataAvailable(&huart2))
		{
			char data = Uart_read(&huart2);
			sprintf(buff,"%c",data);
			strcat(buffTx,buff);
		}

		//check buffer is empty and send it
		if(buffTx[0]!='\0'){

			xQueueSend(simpleQueue,(void*)buffTx,32);
			xQueueSend(simpleQueue4,(void*)buffTx,32);
			//HAL_UART_Transmit(&huart2, (uint8_t *)buffTx, strlen(buffTx), 10);
			vTaskDelay(Tickdelay);
		}
		//reset buffer
		strcpy(buffTx,"\0");

		if (xQueueReceive(simpleQueue3, (void*)buff_Rx, 100) != pdTRUE)
		{
			//HAL_UART_Transmit(&huart2, (uint8_t *)"Error in Receiving from Queue2\n\n", 31, 1000);
		}
		else
		{
			for(int i=0;i<strlen(buff_Rx);i++)
			{
				Uart_write((char)buff_Rx[i], &huart2);
			}
		}

		if (xQueueReceive(simpleQueue2, (void*)buff_Rx2, 100) != pdTRUE)
		{
					//HAL_UART_Transmit(&huart2, (uint8_t *)"Error in Receiving from Queue2\n\n", 31, 1000);
		}
		else
		{
			HAL_UART_Transmit(&huart2, (uint8_t *)buff_Rx2, strlen(buff_Rx2), 10);
		}
		strcpy(buff_Rx,"\0");
		strcpy(buff_Rx2,"\0");
		vTaskDelay(Tickdelay);
	}
}

void taskUartHc12(void* argument)
{
	Ringbuf_init();
	uint32_t Tickdelay = pdMS_TO_TICKS(100);
	char buff[30];
	char buffTx[30]="";
	char buff_Rx[100];
	char data;
	while(1)
	{
		while(IsDataAvailable(&huart1))
		{
			data = Uart_read(&huart1);
			sprintf(buff,"%c",data);
			strcat(buffTx,buff);
		}
		//check buffer is empty and send it
		if(buffTx[0]!='\0')
		{
			xQueueSend(simpleQueue2,(void*)buffTx,32);
			vTaskDelay(Tickdelay);
		}
		//reset buffer
		strcpy(buffTx,"\0");


		if (xQueueReceive(simpleQueue, (void*)buff_Rx, 100) != pdTRUE)
		{
			//HAL_UART_Transmit(&huart2, (uint8_t *)"Error in Receiving from Queue1\n\n", 31, 1000);
		}
		else
		{
			for(int i=0;i<strlen(buff_Rx);i++)
			{
				Uart_write((char)buff_Rx[i], &huart1);
			}
		}

		strcpy(buff_Rx,"\0");
		vTaskDelay(Tickdelay);
	}
}
