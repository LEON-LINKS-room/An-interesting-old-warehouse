/*******************************************************************************
MIT License

Copyright (c) 2022 LEON-LINKS-room

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*******************************************************************************/

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "queue.h"

#include "uart_driver.h"
#include "stdio.h"

extern QueueHandle_t cli_queue;

uint8_t uart1_rxdata = 0;


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == USART1)
    {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;

        //数据入队
        if (xQueueSendToBackFromISR(cli_queue, &uart1_rxdata, &xHigherPriorityTaskWoken) != pdPASS)
        {
            printf("uart1 recv data failed.\r\n");
        }

        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

		HAL_UART_Receive_IT(&huart1, (uint8_t *)&uart1_rxdata, 1);
    }
}
