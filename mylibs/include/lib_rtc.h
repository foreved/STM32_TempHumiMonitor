#ifndef _LIB_RTC_H
#define _LIB_RTC_H

#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_rtc.h"
#include "stm32f1xx_ll_pwr.h"
#include "ff.h"

#define LIB_RTC           RTC         // 使用的 RTC 实例
#define LIB_RTC_CLK_FRE   LSE_VALUE   // RTC 的时钟频率

// RTC 中断配置
#define LIB_RTC_IT_EN               1
#if LIB_RTC_IT_EN
    #define LIB_RTC_IRQ             RTC_IRQn
    #define LIB_RTC_IT_SEC_EN       1     // 使能秒中断
    #define LIB_RTC_IT_SEC_PREEM    0     // 抢占优先级
    #define LIB_RTC_IT_SEC_SUB      0     // 子优先级
    #define Lib_RTC_Handler         RTC_IRQHandler
    extern uint8_t Lib_RTC_IT_SEC_Flag;
#endif


/*
 * @brief   Unix 时间戳类型
 * @note    0x0 表示 1970年1月1日0时0分0秒
*/
#define Lib_RTC_UnixType int32_t

/*
 * @brief   Fat 时间戳类型
 * @note    高16位表示日期 --bit15~bit 9: 年-1908
 *                       --bit 8~bit 5: 月 (1~12)
 *                       --bit 4~bit 0: 日 (1~31)
 *          低16位表示时间 --bit15~bit11: 时 (0~23)
 *                       --bit10~bit 5: 分 (0~59)
 *                       --bit 4~bit 0: 秒/2 (0~29)
*/
#define Lib_RTC_FatType uint32_t 

/*
 * @brief   日期类型
*/
typedef struct {
    int16_t year;     // 1970+
    int8_t month;     // 1-12
    int8_t day;       // 1-31
    int8_t hour;      // 0-23
    int8_t minute;    // 0-59
    int8_t second;    // 0-59
} Lib_RTC_DateType;

/*
 * @brief   所在的 UTC 时区
*/
#define LIB_RTC_TIMEZONE        (+8)
#define LIB_RTC_UNIX        1735689600 

/*
 * @brief   检查两个 Unix 时间戳是否相同
*/
#define Lib_RTC_Cheack_Same_Unix(ts1, ts2) ((ts1) == (ts2))

void Lib_RTC_Init(void);
void Lib_RTC_Set_Time(const Lib_RTC_UnixType ts);
Lib_RTC_UnixType Lib_RTC_Read_Time(void);
Lib_RTC_UnixType Lib_RTC_Date2Unix(const Lib_RTC_DateType *dt);
void Lib_RTC_Unix2Date(const Lib_RTC_UnixType timestamp, Lib_RTC_DateType* dt);
uint8_t Lib_RTC_Check_Same_Date(const Lib_RTC_DateType *const dt1, const Lib_RTC_DateType *const dt2);
Lib_RTC_UnixType Lib_RTC_Fat2Unix(const Lib_RTC_FatType fat);
Lib_RTC_FatType Lib_RTC_Unix2Fat(const Lib_RTC_UnixType ts);
void Lib_RTC_Fat2Date(const Lib_RTC_FatType fat, Lib_RTC_DateType *const dt);
Lib_RTC_FatType Lib_RTC_Date2Fat(const Lib_RTC_DateType* const dt);

#if (!FF_FS_NORTC)
DWORD get_fattime(void);
#endif

#endif