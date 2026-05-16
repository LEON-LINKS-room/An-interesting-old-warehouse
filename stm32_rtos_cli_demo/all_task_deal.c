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
#include "task.h"
#include "queue.h"

#include "all_task_deal.h"
#include "gpio.h"
#include "cli_lite.h"

// 运行指示灯
void runled_task(void *pvParameters){
    for(;;){
        HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_13);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
#define RUNLED_STK_SIZE     configMINIMAL_STACK_SIZE
#define RUNLED_PRIO         1
TaskHandle_t RUNLED_Handler;

// CLI接口
void cli_task(void *pvParameters){
    for(;;){
        cli_deal();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
#define CLI_STK_SIZE     configMINIMAL_STACK_SIZE*2
#define CLI_PRIO         2
TaskHandle_t CLI_Handler;

// 串口队列
QueueHandle_t cli_queue;
#define CLIQUEUE_uxLength     512
#define CLIQUEUE_uxItemSize   1

// 启动所有任务
void start_all_task(void){

    xTaskCreate(runled_task, \
                "runled_task", \
                RUNLED_STK_SIZE, \
                NULL, \
                RUNLED_PRIO, \
                &RUNLED_Handler);
                
    xTaskCreate(cli_task, \
                "cli_task", \
                CLI_STK_SIZE, \
                NULL, \
                CLI_PRIO, \
                &CLI_Handler);

    cli_queue = xQueueCreate(CLIQUEUE_uxLength,CLIQUEUE_uxItemSize);

    vTaskStartScheduler();
}
