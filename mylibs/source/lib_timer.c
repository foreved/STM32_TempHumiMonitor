#include "lib_timer.h" 
#include "mod_dht11.h"

uint8_t LIB_TIMER_CYCLE_FLAG;

void Lib_Timer_Init(void)
{
    LL_TIM_InitTypeDef tim_config;

    Lib_Timer_CLKEN();

    tim_config.CounterMode = LL_TIM_COUNTERMODE_UP;
    tim_config.Prescaler = LIB_TIMER_PRE;
    tim_config.Autoreload = LIB_TIMER_CYCLE;
    LL_TIM_Init(LIB_TIMER, &tim_config);

    LL_TIM_SetOnePulseMode(LIB_TIMER, LL_TIM_ONEPULSEMODE_REPETITIVE);

    NVIC_SetPriority(LIB_TIMER_IRQ, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), LIB_TIMER_IT_PRE, LIB_TIMER_IT_SUB));
    NVIC_EnableIRQ(LIB_TIMER_IRQ);
    LL_TIM_EnableIT_UPDATE(LIB_TIMER);
}


static uint32_t num_ms;
void Lib_Timer_IT_Handler(void)
{
    if (LL_TIM_IsActiveFlag_UPDATE(LIB_TIMER) == SET)
    {
        ++num_ms;
        if (num_ms == 2000)
        {
            num_ms = 0;
            LIB_TIMER_CYCLE_FLAG = 1;
        }
        LL_TIM_ClearFlag_UPDATE(LIB_TIMER);
    }
}