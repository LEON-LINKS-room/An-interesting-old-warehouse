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
