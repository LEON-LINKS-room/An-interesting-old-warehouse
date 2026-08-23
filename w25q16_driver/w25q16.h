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

#ifndef __W25Q16_H
#define __W25Q16_H

#include "gpio.h"

// W25Q16 指令集
#define W25Q16_WRITE_ENABLE 0x06
#define W25Q16_WRITE_DISABLE 0x04
#define W25Q16_READ_STATUS_REG 0x05
#define W25Q16_WRITE_STATUS_REG 0x01
#define W25Q16_READ_DATA 0x03
#define W25Q16_FAST_READ_DATA 0x0B
#define W25Q16_PAGE_PROGRAM 0x02
#define W25Q16_SECTOR_ERASE 0x20
#define W25Q16_CHIP_ERASE 0xC7
#define W25Q16_POWER_DOWN 0xB9
#define W25Q16_RELEASE_POWER_DOWN 0xAB
#define W25Q16_DEVICE_ID 0x90
#define W25Q16_JEDEC_ID 0x9F

// GPIO 控制 CS 引脚
#define W25Q16_CS_LOW() HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET)
#define W25Q16_CS_HIGH() HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET)

void W25Q16_Init(SPI_HandleTypeDef *hspi);
int W25Q16_ReadID(uint8_t rx_buf[4]);
int W25Q16_ReadData(uint32_t addr, uint8_t *data, uint16_t length);
int W25Q16_WriteData(uint32_t addr, uint8_t *data, uint16_t length);

#endif
