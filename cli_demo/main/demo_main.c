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
#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/stream_buffer.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include "esp_log.h"
#include "cli_lite.h"

/* 定义串口参数 */
#define CLI_UART_PORT_NUM      1
#define CLI_UART_BAUD_RATE     115200
#define CLI_TEST_TXD           GPIO_NUM_4
#define CLI_TEST_RXD           GPIO_NUM_5
#define CLI_TEST_RTS           UART_PIN_NO_CHANGE
#define CLI_TEST_CTS           UART_PIN_NO_CHANGE
#define UART_BUF_SIZE          128

/* 定义队列参数 */
QueueHandle_t   Uart_Queue;
#define QUEUE_LENGTH    16

StreamBufferHandle_t Uart_Data_Stream;
#define Data_STREAM_SIZE  128

/* 定义任务参数 */
void uart_task(void *pvParameters);
TaskHandle_t UART_TASK_Handler;
#define UART_TASK_STK_SIZE  4096
#define UART_TASK_PRIO  2

void cli_task(void *pvParameters);
TaskHandle_t CLI_TASK_Handler;
#define CLI_TASK_STK_SIZE  4096
#define CLI_TASK_PRIO  1

portMUX_TYPE main_mux = portMUX_INITIALIZER_UNLOCKED;

/* 启动任务 */
void app_main(void){
    taskENTER_CRITICAL(&main_mux);

    xTaskCreate((TaskFunction_t )uart_task,
                (const char* )"uart_task",
                (uint16_t )UART_TASK_STK_SIZE,
                (void* )NULL,
                (UBaseType_t )UART_TASK_PRIO,
                (TaskHandle_t* )&UART_TASK_Handler);

    xTaskCreate((TaskFunction_t )cli_task,
                (const char* )"cli_task",
                (uint16_t )CLI_TASK_STK_SIZE,
                (void* )NULL,
                (UBaseType_t )CLI_TASK_PRIO,
                (TaskHandle_t* )&CLI_TASK_Handler);
    
    Uart_Data_Stream = xStreamBufferCreate(Data_STREAM_SIZE, 1);

    vTaskDelete(NULL);

    taskEXIT_CRITICAL(&main_mux);
}

/* 串口接收任务 */
void uart_task(void *pvParameters){

    uart_event_t event;
    uint8_t data[128] = {0};

    uart_config_t uart_config = {
        .baud_rate = CLI_UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    int intr_alloc_flags = 0;

#if CONFIG_UART_ISR_IN_IRAM
    intr_alloc_flags = ESP_INTR_FLAG_IRAM;
#endif
    //安装串口驱动
    ESP_ERROR_CHECK(uart_driver_install(
        CLI_UART_PORT_NUM,
        2*UART_BUF_SIZE,
        2*UART_BUF_SIZE,
        QUEUE_LENGTH,
        &Uart_Queue,
        intr_alloc_flags
    ));
    //串口配置
    ESP_ERROR_CHECK(uart_param_config(CLI_UART_PORT_NUM, &uart_config));
    //引脚配置
    ESP_ERROR_CHECK(uart_set_pin(
        CLI_UART_PORT_NUM,
        CLI_TEST_TXD,
        CLI_TEST_RXD,
        CLI_TEST_RTS,
        CLI_TEST_CTS
    ));

    for(;;){
        if(xQueueReceive(Uart_Queue, &event, portMAX_DELAY)){
            switch (event.type){
                //数据
                case UART_DATA:{
                    int datalen = event.size;
                    if(datalen>128){
                        datalen = 128;
                    }
                    int len = uart_read_bytes(CLI_UART_PORT_NUM, data, datalen, portMAX_DELAY);
                    if (len > 0){
                        xStreamBufferSend(Uart_Data_Stream, data, len, portMAX_DELAY);
                    }
                }break;
                //FIFO溢出
                case UART_FIFO_OVF:{
                    ESP_LOGW("UART", "FIFO overflow");
                    uart_flush_input(CLI_UART_PORT_NUM);
                    xQueueReset(Uart_Queue);
                }break;

                default:break;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

/* 命令行处理任务 */
void cli_task(void *pvParameters){
    uint8_t buf[128] = {0};
    for(;;){
        size_t len = xStreamBufferReceive(Uart_Data_Stream,buf,sizeof(buf),portMAX_DELAY);
        for (int i = 0; i < len; i++){
            uint8_t ch = buf[i];
            cli_deal(ch);
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
