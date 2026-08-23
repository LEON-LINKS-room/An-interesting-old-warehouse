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

#include "w25q16.h"
#include "stdio.h"
#include "string.h"

static SPI_HandleTypeDef *w25q16_hspi;

// 初始化
void W25Q16_Init(SPI_HandleTypeDef *hspi)
{
    w25q16_hspi = hspi;
    W25Q16_CS_HIGH(); // 默认不选中
}

// 读 flash id
int W25Q16_ReadID(uint8_t rx_buf[4])
{
    int ret = HAL_OK;
    uint8_t tx_cmd = W25Q16_JEDEC_ID; // JEDEC ID命令
    uint8_t dummy[3] = {0xff, 0xff, 0xff};

    W25Q16_CS_LOW();
    ret = HAL_SPI_Transmit(w25q16_hspi, &tx_cmd, 1, HAL_MAX_DELAY); // 发送读ID命令
    if (ret != HAL_OK)
    {
        W25Q16_CS_HIGH();
        return HAL_ERROR;
    }
    // 读取3个字节（发送dummy数据）
    ret = HAL_SPI_TransmitReceive(w25q16_hspi, dummy, rx_buf, 3, HAL_MAX_DELAY);
    if (ret != HAL_OK)
    {
        W25Q16_CS_HIGH();
        return HAL_ERROR;
    }
    W25Q16_CS_HIGH();

    return HAL_OK;
}

// 读数据
int W25Q16_ReadData(uint32_t addr, uint8_t *data, uint16_t length)
{
    int ret = HAL_OK;
    uint32_t timeout = 0U;
    uint8_t busy_status[2] = {0x0, 0x1};
    uint8_t cmd_rdstatus = W25Q16_READ_STATUS_REG; // 读状态寄存器

    uint8_t rd_cmd[4] = {// 读命令和地址
                         W25Q16_READ_DATA,
                         (addr >> 16) & 0xFF,
                         (addr >> 8) & 0xFF,
                         addr & 0xFF};

    if (addr & 0xFFF)
    {
        printf("Error: address 0x%08X is not 4K aligned \r\n", addr);
        return HAL_ERROR;
    }

    timeout = HAL_GetTick() + 1000; // 超时
    do
    {
        W25Q16_CS_LOW();
        ret = HAL_SPI_Transmit(w25q16_hspi, &cmd_rdstatus, 1, HAL_MAX_DELAY);
        if (ret != HAL_OK)
        {
            W25Q16_CS_HIGH();
            return HAL_ERROR;
        }
        for (int i = 0; i < 2; i++)
        {
            uint8_t dummy = 0xFF;
            ret = HAL_SPI_TransmitReceive(w25q16_hspi, &dummy, &busy_status[i], 1, HAL_MAX_DELAY);
            if (ret != HAL_OK)
            {
                W25Q16_CS_HIGH();
                return HAL_ERROR;
            }
        }
        W25Q16_CS_HIGH();

        if (HAL_GetTick() > timeout)
        {
            W25Q16_CS_HIGH();
            printf("Erase timeout!\r\n");
            return HAL_ERROR;
        }
        HAL_Delay(10); // 避免过于频繁查询
    } while (busy_status[1] & 0x01); // BUSY位

    W25Q16_CS_LOW();
    ret = HAL_SPI_Transmit(w25q16_hspi, rd_cmd, 4, HAL_MAX_DELAY); // 发送读命令
    if (ret != HAL_OK)
    {
        W25Q16_CS_HIGH();
        return HAL_ERROR;
    }
    // 读数据
    for (int i = 0; i < length; i++)
    {
        uint8_t dummy = 0xFF;
        ret = HAL_SPI_TransmitReceive(w25q16_hspi, &dummy, &data[i], 1, HAL_MAX_DELAY);
        if (ret != HAL_OK)
        {
            W25Q16_CS_HIGH();
            return HAL_ERROR;
        }
    }
    W25Q16_CS_HIGH();

    return HAL_OK;
}

// 写数据
int W25Q16_WriteData(uint32_t addr, uint8_t *data, uint16_t length)
{
    int ret = HAL_OK;
    uint8_t busy_status[2] = {0x0, 0x1};
    uint32_t timeout = 0U;

    uint8_t cmd_enable = W25Q16_WRITE_ENABLE;      // 使能写
    uint8_t cmd_disable = W25Q16_WRITE_DISABLE;    // 去使能写
    uint8_t cmd_rdstatus = W25Q16_READ_STATUS_REG; // 读状态寄存器

    uint8_t cmd_er[4] = {// 擦除命令
                         W25Q16_SECTOR_ERASE,
                         (addr >> 16) & 0xFF,
                         (addr >> 8) & 0xFF,
                         addr & 0xFF};

    uint8_t cmd_wr[4] = {// 写命令
                         W25Q16_PAGE_PROGRAM,
                         (addr >> 16) & 0xFF,
                         (addr >> 8) & 0xFF,
                         addr & 0xFF};

    unsigned int Page_Num = (length / 0x100) + 1; // 总共需写多少个page
    length = (length % 0x100);

    if (addr & 0xFFF)
    {
        printf("Error: address 0x%08X is not 4K aligned \r\n", addr);
        return HAL_ERROR;
    }

    W25Q16_CS_LOW();
    ret = HAL_SPI_Transmit(w25q16_hspi, &cmd_enable, 1, HAL_MAX_DELAY); // 使能写
    if (ret != HAL_OK)
    {
        W25Q16_CS_HIGH();
        return HAL_ERROR;
    }
    W25Q16_CS_HIGH();

    W25Q16_CS_LOW();
    ret = HAL_SPI_Transmit(w25q16_hspi, cmd_er, 4, HAL_MAX_DELAY); // 写之前先擦除
    if (ret != HAL_OK)
    {
        W25Q16_CS_HIGH();
        return HAL_ERROR;
    }
    W25Q16_CS_HIGH();

    timeout = HAL_GetTick() + 1000; // 超时
    do
    {
        W25Q16_CS_LOW();
        ret = HAL_SPI_Transmit(w25q16_hspi, &cmd_rdstatus, 1, HAL_MAX_DELAY);
        if (ret != HAL_OK)
        {
            W25Q16_CS_HIGH();
            return HAL_ERROR;
        }
        for (int i = 0; i < 2; i++)
        {
            uint8_t dummy = 0xFF;
            ret = HAL_SPI_TransmitReceive(w25q16_hspi, &dummy, &busy_status[i], 1, HAL_MAX_DELAY);
            if (ret != HAL_OK)
            {
                W25Q16_CS_HIGH();
                return HAL_ERROR;
            }
        }
        W25Q16_CS_HIGH();

        if (HAL_GetTick() > timeout)
        {
            W25Q16_CS_HIGH();
            printf("Erase timeout!\r\n");
            return HAL_ERROR;
        }
        HAL_Delay(10); // 避免过于频繁查询
    } while (busy_status[1] & 0x01); // BUSY位

    for (int page = 0; page < Page_Num; page++)
    {
        cmd_wr[1] = ((addr + 0x100 * page) >> 16) & 0xFF;
        cmd_wr[2] = ((addr + 0x100 * page) >> 8) & 0xFF;
        cmd_wr[3] = (addr + 0x100 * page) & 0xFF;
        W25Q16_CS_LOW();
        ret = HAL_SPI_Transmit(w25q16_hspi, &cmd_enable, 1, HAL_MAX_DELAY); // 再次使能写
        if (ret != HAL_OK)
        {
            W25Q16_CS_HIGH();
            return HAL_ERROR;
        }
        W25Q16_CS_HIGH();

        W25Q16_CS_LOW();
        ret = HAL_SPI_Transmit(w25q16_hspi, cmd_wr, 4, HAL_MAX_DELAY); // 发送写命令
        if (ret != HAL_OK)
        {
            W25Q16_CS_HIGH();
            return HAL_ERROR;
        }
        if (page == Page_Num - 1) // 若最后一个page
        {
            ret = HAL_SPI_Transmit(w25q16_hspi, data + 0x100 * page, length, HAL_MAX_DELAY); // 发送写命令
        }
        else
        {
            ret = HAL_SPI_Transmit(w25q16_hspi, data + 0x100 * page, 0x100, HAL_MAX_DELAY); // 发送写命令
        }
        if (ret != HAL_OK)
        {
            W25Q16_CS_HIGH();
            return HAL_ERROR;
        }
        W25Q16_CS_HIGH();
        timeout = HAL_GetTick() + 1000; // 超时
        do
        {
            W25Q16_CS_LOW();
            ret = HAL_SPI_Transmit(w25q16_hspi, &cmd_rdstatus, 1, HAL_MAX_DELAY);
            if (ret != HAL_OK)
            {
                W25Q16_CS_HIGH();
                return HAL_ERROR;
            }
            for (int i = 0; i < 2; i++)
            {
                uint8_t dummy = 0xFF;
                ret = HAL_SPI_TransmitReceive(w25q16_hspi, &dummy, &busy_status[i], 1, HAL_MAX_DELAY);
                if (ret != HAL_OK)
                {
                    W25Q16_CS_HIGH();
                    return HAL_ERROR;
                }
            }
            W25Q16_CS_HIGH();

            if (HAL_GetTick() > timeout)
            {
                W25Q16_CS_HIGH();
                printf("Erase timeout!\r\n");
                return HAL_ERROR;
            }
            HAL_Delay(10); // 避免过于频繁查询
        } while (busy_status[1] & 0x01); // BUSY位
    }

    W25Q16_CS_LOW();
    ret = HAL_SPI_Transmit(w25q16_hspi, &cmd_disable, 1, HAL_MAX_DELAY); // 写完去使能写
    if (ret != HAL_OK)
    {
        W25Q16_CS_HIGH();
        return HAL_ERROR;
    }
    W25Q16_CS_HIGH();

    return HAL_OK;
}
