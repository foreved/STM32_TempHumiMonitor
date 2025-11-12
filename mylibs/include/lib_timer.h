#ifndef _LIB_TIMER_H
#define _LIB_TIMER_H

#include "stm32f1xx_ll_rcc.h"
#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_tim.h"

/*
 * @brief   计时器配置 -- 定时任务
 *          时基: 1us
*/
#define LIB_TIMER TIM2
#define Lib_Timer_CLKEN() LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM2)
#define LIB_TIMER_CLK  72000000
#define LIB_TIMER_PRE  (LIB_TIMER_CLK / 1000000 - 1)
#define LIB_TIMER_CYCLE (1000 - 1) // 1ms 周期
#define LIB_TIMER_IRQ TIM2_IRQn
#define LIB_TIMER_IT_PRE 0
#define LIB_TIMER_IT_SUB 0
#define Lib_Timer_IT_Handler TIM2_IRQHandler
extern uint8_t LIB_TIMER_CYCLE_FLAG;

void Lib_Timer_Init(void);

#endif