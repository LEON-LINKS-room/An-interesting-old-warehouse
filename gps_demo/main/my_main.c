/*******************************************************************************
MIT License

Copyright (c) 2023 LEON-LINKS-room

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
#include "nvs_flash.h"
#include "esp_system.h"
#include "esp_log.h"
#include "gps_deal.h"
#include "web_server.h"

#define GPS_UART_PORT_NUM       2
#define GPS_UART_BAUD_RATE      9600
#define GPS_GPIO_UART_TXD       GPIO_NUM_17
#define GPS_GPIO_UART_RXD       GPIO_NUM_16
#define GPS_GPIO_UART_RTS       UART_PIN_NO_CHANGE
#define GPS_GPIO_UART_CTS       UART_PIN_NO_CHANGE
#define GPS_UART_BUF_SIZE       512

QueueHandle_t       GPS_Uart_Queue;
#define GPS_QUEUE_LENGTH    16

StreamBufferHandle_t GPS_Uart_Data_Stream;
#define GPS_Data_STREAM_SIZE  512

void uart2_task(void *pvParameters);
TaskHandle_t UART2_TASK_Handler;
#define UART2_TASK_STK_SIZE  4096
#define UART2_TASK_PRIO  2

void gps_task(void *pvParameters);
TaskHandle_t GPS_TASK_Handler;
#define GPS_TASK_STK_SIZE  4096
#define GPS_TASK_PRIO  1

portMUX_TYPE main_mux = portMUX_INITIALIZER_UNLOCKED;
SemaphoreHandle_t gps_binary_semaphore;

void app_main(void){
    taskENTER_CRITICAL(&main_mux);

    xTaskCreatePinnedToCore((TaskFunction_t )uart2_task,
                (const char* )"uart2_task",
                (uint16_t )UART2_TASK_STK_SIZE,
                (void* )NULL,
                (UBaseType_t )UART2_TASK_PRIO,
                (TaskHandle_t* )&UART2_TASK_Handler,
                APP_CPU_NUM);

    xTaskCreatePinnedToCore((TaskFunction_t )gps_task,
                (const char* )"gps_task",
                (uint16_t )GPS_TASK_STK_SIZE,
                (void* )NULL,
                (UBaseType_t )GPS_TASK_PRIO,
                (TaskHandle_t* )&GPS_TASK_Handler,
                APP_CPU_NUM);

    taskEXIT_CRITICAL(&main_mux);

    GPS_Uart_Data_Stream = xStreamBufferCreate(GPS_Data_STREAM_SIZE, 1);

    gps_binary_semaphore = xSemaphoreCreateBinary();
    xSemaphoreGive(gps_binary_semaphore);

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND){
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    wifi_init_softap();
    start_web_server();

    vTaskDelete(NULL);
}

void uart2_task(void *pvParameters){
    uart_event_t event;
    uint8_t data[512] = {0};

    uart_config_t uart_config = {
        .baud_rate = GPS_UART_BAUD_RATE,
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

    ESP_ERROR_CHECK(uart_driver_install(
        GPS_UART_PORT_NUM,
        2*GPS_UART_BUF_SIZE,
        2*GPS_UART_BUF_SIZE,
        GPS_QUEUE_LENGTH,
        &GPS_Uart_Queue,
        intr_alloc_flags
    ));

    ESP_ERROR_CHECK(uart_param_config(GPS_UART_PORT_NUM, &uart_config));

    ESP_ERROR_CHECK(uart_set_pin(
        GPS_UART_PORT_NUM,
        GPS_GPIO_UART_TXD,
        GPS_GPIO_UART_RXD,
        GPS_GPIO_UART_RTS,
        GPS_GPIO_UART_CTS
    ));

    for(;;){
        if(xQueueReceive(GPS_Uart_Queue, &event, pdMS_TO_TICKS(500))){
            switch (event.type){

                case UART_DATA:{
                    int datalen = event.size;
                    if(datalen>(sizeof(data)-1)){
                        datalen = (sizeof(data)-1);
                    }
                    int len = uart_read_bytes(GPS_UART_PORT_NUM, data, datalen, pdMS_TO_TICKS(100));
                    if (len > 0){
                        xStreamBufferSend(GPS_Uart_Data_Stream, data, len, 0);
                    }
                }break;

                case UART_FIFO_OVF:{
                    ESP_LOGW("UART2", "FIFO overflow");
                    uart_flush_input(GPS_UART_PORT_NUM);
                    xQueueReset(GPS_Uart_Queue);
                }break;

                default: break;
            }
        }
        ESP_LOGI("INFO", "uart2_task");
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

void gps_task(void *pvParameters){
    uint8_t buf[512] = {0};
    for(;;){
        if(xSemaphoreTake(gps_binary_semaphore,pdMS_TO_TICKS(200))==pdTRUE){
            size_t len = xStreamBufferReceive(GPS_Uart_Data_Stream,buf,sizeof(buf),pdMS_TO_TICKS(100));
            for (int i = 0; i < len; i++){
                uint8_t ch = buf[i];
                gps_deal(ch);
            }
            xSemaphoreGive(gps_binary_semaphore);
        }
        ESP_LOGI("INFO", "gps_task");
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}
