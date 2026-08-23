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

#ifndef __INA219_H
#define __INA219_H

#include "stm32f1xx_hal.h"
#include "i2c.h"

#define INA219_ADDRESS (0x40 << 1) // Ä¬ÈÏI2CµØÖ·

extern I2C_HandleTypeDef hi2c2;

void INA219_Init(void);
float INA219_ReadShuntVoltage_V(void);
float INA219_ReadBusVoltage_V(void);
float INA219_ReadPower_W(void);
float INA219_ReadCurrent_A(void);
float INA219_ReadTotalVoltage_V(void);

#endif
