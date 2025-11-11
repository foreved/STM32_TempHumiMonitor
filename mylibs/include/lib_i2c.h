#ifndef _LIB_I2C_H
#define _LIB_I2C_H

#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_i2c.h"
#include "stm32f1xx_ll_gpio.h"

// I2C配置
#define    LIB_I2C                       I2C1   // 使用I2C1
#define    LIB_I2C_ENCLK()               LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I2C1)
#define    LIB_I2C_SPEED                 400000 // SCL时钟频率，必须不高于400kHz
#define    LIB_I2C_ADDR                  0x01   // I2C的地址，必须是唯一的，且在[0x00, 0x3FF]

// GPIO配置
#define    LIB_I2C_SCL_PORT      GPIOB             // I2C1_SCL为PB6
#define    LIB_I2C_SCL_PIN       LL_GPIO_PIN_6
#define    LIB_I2C_SDA_PORT      GPIOB             // I2C1_SDA为PB7
#define    LIB_I2C_SDA_PIN       LL_GPIO_PIN_7
#define    LIB_I2C_PORT_ENCLK()  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOB)

void Lib_I2C_Init(void);
void Lib_I2C_Send_Data(const uint8_t slave_addr, const uint8_t *const buffer, const uint32_t num);
void Lib_I2C_Receive_Data(const uint8_t slave_addr, uint8_t *const buffer, const uint32_t num);

#endif