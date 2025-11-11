#ifndef _LIB_USART_H
#define _LIB_USART_H

#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_gpio.h"
#include "stm32f1xx_ll_usart.h"
#include "stm32f1xx_ll_dma.h"

// USART配置
#define LIB_USART                USART1   // 使用USART1
#define LIB_USART_ENCLK()        LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART1) // 使能USART1的时钟
#define LIB_USART_BAUD_RATE      115200   // 波特率为115.2Kbps

// GPIO配置
#define LIB_USART_TX_PORT        GPIOA    // USART1的TX为PA9
#define LIB_USART_TX_PIN         LL_GPIO_PIN_9
#define LIB_USART_RX_PORT        GPIOA    // USART1的RX为PA10
#define LIB_USART_RX_PIN         LL_GPIO_PIN_10
#define LIB_USART_PORT_ENCLK()   LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOA) // 使能GPIOA的时钟

// USART的中断配置
#define LIB_USART_IT_EN              0    // 是否启用中断
#if LIB_USART_IT_EN
    #define LIB_USART_IRQ                USART1_IRQn  // USART1的中断编号
    #define LIB_USART_PREEMPT_PRIORITY   0    // 抢占优先级
    #define LIB_USART_SUB_PRIORITY       0    // 子优先级
    #define LIB_USART_IT_RX_EN           1    // 是否启用接收中断
    #define Lib_USART_IT_Handler         USART1_IRQHandler  // USART1的中断服务函数
    #define LIB_USART_BUFFER_MAXSIZE        100
    extern uint8_t Lib_USART_Buffer[LIB_USART_BUFFER_MAXSIZE];  // USART的缓冲区, 需在main.c中定义为全局便量
#endif

// DMA配置
#define LIB_USART_DMA_EN             0        // 是否使用DMA
#if LIB_USART_DMA_EN
    #define LIB_USART_DMA                DMA1
    #define LIB_USART_DMA_CH             LL_DMA_CHANNEL_4     // USART1_TX对应通道
    #define LIB_USART_DMA_CH_EN()        LL_DMA_EnableChannel(LIB_USART_DMA, LIB_USART_DMA_CH)
    #define LIB_USART_DMA_ENCLK()        LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1)
    #define LIB_USART_DMA_PRIORITY       LL_DMA_PRIORITY_VERYHIGH
    #define LIB_USART_DMA_DIRECTION      LL_DMA_DIRECTION_MEMORY_TO_PERIPH
    #define LIB_USART_DMA_MODE           LL_DMA_MODE_NORMAL
    #define LIB_USART_DMA_PADDR          (uint32_t)(&LIB_USART->DR) // 外设地址; 内存源地址
    #define LIB_USART_DMA_MADDR          (uint32_t)Lib_USART_Buffer // 内存地址; 内存目标地址
    #define LIB_USART_DMA_PINC           LL_DMA_PERIPH_NOINCREMENT  // 外设指针自增; 内存源指针自增
    #define LIB_USART_DMA_MINC           LL_DMA_MEMORY_INCREMENT    // 内存指针自增; 内存目标指针自增
    #define LIB_USART_DMA_PDSIZE         LL_DMA_PDATAALIGN_BYTE     // 外设数据大小; 内存源数据大小
    #define LIB_USART_DMA_MDSIZE         LL_DMA_PDATAALIGN_BYTE     // 内存数据大小; 内存目标数据大小
    #define LIB_USART_DMA_NDATA          LIB_USART_BUFFER_MAXSIZE   // 输出数据的数量
#endif

// 指令
#define LIB_USART_CMD_EN                 0         // 是否启用指令模式
#if LIB_USART_CMD_EN
    #include "lib_rtc.h"
    extern uint8_t Lib_USART_CMD_Status1;
    extern uint8_t Lib_USART_CMD_Status2;
    extern uint32_t Lib_USART_CMD_Data;
    #define LIB_USART_CMD_ACK                0xFF      // mcu 回复 pc
    #define LIB_USART_CMD_START              0x01      // 开启指令模式
    #define LIB_USART_CMD_STOP               0x02      // 关闭指令模式
    #define LIB_USART_CMD_RTC_UNIX           0x03      // 配置 RTC 时间戳
#endif

void Lib_USART_Init(void);
void Lib_USART_Send_Byte(const uint8_t data);
void Lib_USART_Send_String(const char *str);
void Lib_USART_Int2Char_DEC(const int num, uint8_t *const buffer);
void Lib_USART_Int2Char_HEX(const int num, uint8_t *const buffer);
void Lib_USART_Double2Char(const double num, uint8_t *const buffer, const uint8_t num_frac_bits);
void Lib_USART_IT_Handler(void);

#endif