#ifndef _LIB_TOOL_H
#define _LIB_TOOL_H

#include "stm32f1xx_ll_cortex.h"

// AHB 时钟频率
#define LIB_TOOL_AHB_FREQUENCY     72000000

/*
 * @brief   使用 DWT 实现计时器, 开始计时, 与 Lib_Tool_DWT_Timer_End() 一起使用
 * @param   无
 * @return  uint32_t
 * @note    推荐场景: 高精度, 微妙级或毫秒级, 短期计时
*/
#define    Lib_Tool_DWT_Timer_Start()    (DWT->CYCCNT)

#define Lib_Tool_Abs(num) (num < 0 ? -num : num)
void Lib_Tool_SysTick_Init(void);
void Lib_Tool_SysTick_Delay_ms(const uint16_t num_ms);
void Lib_Tool_DWT_Init(void);
void Lib_Tool_DWT_Delay_us(const uint16_t num_us);
uint32_t Lib_Tool_DWT_Timer_End(const uint32_t start, const uint8_t is_us);
const char* const Lib_Tool_Format_String(const char *str, ...);
const uint8_t Lib_Tool_Int2Char_DEC(const int num, char *buffer);
const uint8_t Lib_Tool_Int2Char_HEX(const int num, char *buffer);
const uint8_t Lib_Tool_Float2Char(const float num, char *buffer, const uint8_t num_frac_bits);

#endif