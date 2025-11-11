#ifndef _LIB_SPI_H
#define _LIB_SPI_H

#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_gpio.h"
#include "stm32f1xx_ll_spi.h"

// SPI 配置
#define LIB_SPI_ENCLK()                 LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SPI1)
#define LIB_SPI                         SPI1                                                    // 使用SPI1
#define LIB_SPI_CPOL                    LL_SPI_POLARITY_LOW                                     // 模式 0 的 CPOL 和 CPHA 都是 0
#define LIB_SPI_CPHA                    LL_SPI_PHASE_1EDGE
#define LIB_SPI_BIT_ORDER               LL_SPI_MSB_FIRST                                        // MSB 先发送
#define LIB_SPI_BAUD_RATE               LL_SPI_BAUDRATEPRESCALER_DIV2                           // f_SCK = f_pclk / 2

// GPIO配置             
#define LIB_SPI_PORT_ENCLK()            LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOA) 
#define LIB_SPI_NSS_PORT                GPIOA                                                   // SPI1_NSS 为 PA4
#define LIB_SPI_NSS_PIN                 LL_GPIO_PIN_4                                         
#define LIB_SPI_SCK_PORT                GPIOA                                                   // SPI1_SCK 为 PA5
#define LIB_SPI_SCK_PIN                 LL_GPIO_PIN_5                                         
#define LIB_SPI_MISO_PORT               GPIOA                                                   // SPI1_MISO 为 PA6
#define LIB_SPI_MISO_PIN                LL_GPIO_PIN_6                                         
#define LIB_SPI_MOSI_PORT               GPIOA                                                   // SPI1_MOSI 为 PA7
#define LIB_SPI_MOSI_PIN                LL_GPIO_PIN_7

// SPI 控制
#define LIB_SPI_START()                 LL_GPIO_ResetOutputPin(LIB_SPI_NSS_PORT, LIB_SPI_NSS_PIN)   // NSS 低电平表示通信开始
#define LIB_SPI_STOP()                  LL_GPIO_SetOutputPin(LIB_SPI_NSS_PORT, LIB_SPI_NSS_PIN)     // NSS 高电平表示通信结束
#define LIB_SPI_DUMMY                   0x00                                                        // 无效数据, 用于等待或接收

void Lib_SPI_Init(void);
uint8_t Lib_SPI_Send_Byte(uint8_t data);
uint8_t Lib_SPI_Receive_Byte(void);

#endif