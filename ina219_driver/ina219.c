#include "ina219.h"

// 写INA219寄存器
static void INA219_WriteRegister(uint8_t reg, uint16_t value)
{
    uint8_t data[2];
    data[0] = (value >> 8) & 0xFF;
    data[1] = value & 0xFF;
    HAL_I2C_Mem_Write(&hi2c2, INA219_ADDRESS, reg, I2C_MEMADD_SIZE_8BIT, data, 2, HAL_MAX_DELAY);
}

// 读INA219寄存器
static uint16_t INA219_ReadRegister(uint8_t reg)
{
    uint8_t data[2];
    HAL_I2C_Mem_Read(&hi2c2, INA219_ADDRESS, reg, I2C_MEMADD_SIZE_8BIT, data, 2, HAL_MAX_DELAY);
    return ((uint16_t)data[0] << 8) | data[1];
}

// 初始化
void INA219_Init(void)
{
    // 默认设置 Bus Voltage Range:32V; PGA:1/8; BADC: 12bit; SADC: 12bit; MODE: Shunt and bus, continuous
    INA219_WriteRegister(0x00, 0x399F);
    // 校准 最大测量电流3.2A 分流电阻为0.1欧 校准值得: 4194=0x1062
    INA219_WriteRegister(0x05, 0x1062);
}

// 读 ShuntVoltage SADC IN+ to IN-
float INA219_ReadShuntVoltage_V(void)
{
    int16_t raw = (int16_t)INA219_ReadRegister(0x01);
    return (float)raw * 0.00001f; // 10 uV/bit
}

// 读 BusVoltage BADC IN- to GND
float INA219_ReadBusVoltage_V(void)
{
    uint16_t raw = INA219_ReadRegister(0x02);
    uint16_t bus_raw = raw >> 3;
    return (float)bus_raw * 0.004f; // 4 mV/bit
}

// 读 Power
float INA219_ReadPower_W(void)
{
    uint16_t raw = INA219_ReadRegister(0x03);
    return (float)raw * 0.00195312f; // 2 mW/bit
}

// 读 Current
float INA219_ReadCurrent_A(void)
{
    int16_t raw = (int16_t)INA219_ReadRegister(0x04);
    return (float)raw * 0.00009766f; // 100 uA/bit
}

// 读电源电压(Battery Voltage)
float INA219_ReadTotalVoltage_V(void)
{
    return INA219_ReadShuntVoltage_V() + INA219_ReadBusVoltage_V();
}
