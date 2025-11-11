#include "lib_rtc.h"

// 任何修改 RTC 的操作必须等待上一个操作完成, 即 RTOF 被置位
#define LIB_RTC_WAIT_TASK()     do {} while(LL_RTC_IsActiveFlag_RTOF(LIB_RTC) != SET)

/*
 * @brief   初始化 RTC
*/
void Lib_RTC_Init(void)
{   
    // RTC 位于 BKP 区, 所以使能 BKP
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_BKP);
    // 复位后, BKP 默认不能访问, 必须开启访问权限
    LL_PWR_EnableBkUpAccess();
    // RTC 与 APB1 总线分离, 首次访问 RTC 前, 需要同步
    LL_RTC_WaitForSynchro(LIB_RTC);
    
    // 配置 RTC
    // 任何写 RTC 寄存器的前后, 都要确保上次和本次任务已经完成

    // 进行配置前, 必须进入配置模式
    LIB_RTC_WAIT_TASK();
    LL_RTC_DisableWriteProtection(LIB_RTC);
    LIB_RTC_WAIT_TASK();
    // 设置 RTC 预分频系数, 使分频后的 RTC 时钟为 1 Hz
    LL_RTC_SetAsynchPrescaler(LIB_RTC, LIB_RTC_CLK_FRE - 1);
    LIB_RTC_WAIT_TASK();
    // RTC 时间
    LL_RTC_TIME_Set(LIB_RTC, LIB_RTC_UNIX);
    LIB_RTC_WAIT_TASK();

    // RTC 中断
    #if LIB_RTC_IT_EN
        NVIC_SetPriority(LIB_RTC_IRQ, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), LIB_RTC_IT_SEC_PREEM, LIB_RTC_IT_SEC_SUB));
        NVIC_EnableIRQ(LIB_RTC_IRQ);
        #if LIB_RTC_IT_SEC_EN // 秒中断
            LL_RTC_EnableIT_SEC(LIB_RTC);
            LIB_RTC_WAIT_TASK();
        #endif
    #endif

    // 若要使配置生效, 必须退出配置模式
    LL_RTC_EnableWriteProtection(LIB_RTC);
    LIB_RTC_WAIT_TASK();
}

/*
 * @brief   使用 Unix 时间戳设置计数器
 * @param   ts Unix 时间戳
*/
void Lib_RTC_Set_Time(const Lib_RTC_UnixType ts)
{
    // 进行配置前, 必须进入配置模式
    LIB_RTC_WAIT_TASK();
    LL_RTC_DisableWriteProtection(LIB_RTC);
    LIB_RTC_WAIT_TASK();
    // RTC 时间
    LL_RTC_TIME_Set(LIB_RTC, LIB_RTC_UNIX);
    LIB_RTC_WAIT_TASK();
    // 若要使配置生效, 必须退出配置模式
    LL_RTC_EnableWriteProtection(LIB_RTC);
    LIB_RTC_WAIT_TASK();
}

/*
 * @brief   读取现在的 RTC 时间
 * @return  返回数据为 Unix 时间戳
*/
Lib_RTC_UnixType Lib_RTC_Read_Time(void)
{
    LL_RTC_WaitForSynchro(LIB_RTC);
    return (Lib_RTC_UnixType)LL_RTC_TIME_Get(LIB_RTC);
}

// RTC 中断函数
#if LIB_RTC_IT_EN
    uint8_t Lib_RTC_IT_SEC_Flag;

    void Lib_RTC_Handler(void)
    {
        if (LL_RTC_IsActiveFlag_SEC(LIB_RTC) == SET)
        {
            Lib_RTC_IT_SEC_Flag = SET;
            LL_RTC_ClearFlag_SEC(LIB_RTC);
            LIB_RTC_WAIT_TASK();
        }
    }
#endif


static const int days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
#define is_leap_year(year)  ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0))

/*
 * @brief   日期转 Unix 时间戳
 * @param   dt 日期
 * @return  对应的 Unix 时间戳
*/
Lib_RTC_UnixType Lib_RTC_Date2Unix(const Lib_RTC_DateType *dt)
{
    int64_t total_days = 0, res = 0;

    if (dt->year >= 1970)
    {
        for (uint16_t y = 1970; y < dt->year; ++y)
            total_days += is_leap_year(y) ? 366 : 365;
    }
    else
    {
        for (uint16_t y = dt->year; y < 1970; ++y)
            total_days -= is_leap_year(y) ? 366 : 365;
    }
    for (uint8_t m = 1; m < dt->month; ++m)
        total_days += days_in_month[m - 1] + ((m == 2 && is_leap_year(dt->year)) ? 1 : 0);
    total_days += dt->day - 1;
    res = total_days * 86400
        + (int64_t)dt->hour * 3600
        + (int64_t)dt->minute * 60
        + (int64_t)dt->second
        - (int64_t)LIB_RTC_TIMEZONE * 3600;

    if (res < INT32_MIN)
        return INT32_MIN;
    else if (res > INT32_MAX)
        return INT32_MAX;
    else
        return (Lib_RTC_UnixType)res;
}

/*
 * @brief   Unix 时间戳转日期
 * @param   ts Unix 时间戳
 *          dt 相应的日期
*/
void Lib_RTC_Unix2Date(const Lib_RTC_UnixType ts, Lib_RTC_DateType* const dt)
{
    int64_t modified_ts = (int64_t)ts + LIB_RTC_TIMEZONE * 3600;
    int64_t total_days = modified_ts / 86400, secs_of_day = modified_ts % 86400;

    if (secs_of_day < 0)
    {
        secs_of_day += 86400;
        total_days -= 1;
    }
    dt->hour   = secs_of_day / 3600;
    dt->minute = (secs_of_day % 3600) / 60;
    dt->second = secs_of_day % 60;
    for (int32_t year = 1970; ;)
    {
        int32_t days_this_year = is_leap_year(year) ? 366 : 365;
        if (total_days >= days_this_year) 
        {
            total_days -= days_this_year;
            ++year;
        }
        else if (total_days < 0) 
        {   
            total_days += days_this_year;
            --year;
        }
        else 
        {
            dt->year = year;
            break;
        }
    }
    for (uint32_t month = 1; month <= 12; month++) 
    {
        int32_t tmp = days_in_month[month - 1];
        if (month == 2 && is_leap_year(dt->year))
            tmp += 1;
        if (total_days >= tmp)
            total_days -= tmp;
        else
        {
            dt->month = month;
            break;
        }
    }

    dt->day = total_days + 1;
}

/*
 * @brief   检查两个日期是否是同一天
 * @param   dt1, dt2 两个日期
 * @return  0: 不是同一天; 1: 是同一天.
*/
uint8_t Lib_RTC_Check_Same_Date(const Lib_RTC_DateType* const dt1, const Lib_RTC_DateType* const dt2)
{
    if (dt1->year != dt2->year) return 0;
    if (dt1->month != dt2->month) return 0;
    if (dt1->day != dt2->day) return 0;
    if (dt1->hour != dt2->hour) return 0;
    if (dt1->minute != dt2->minute) return 0;
    if (dt1->second != dt2->second) return 0;
    return 1;
}

/*
 * @brief   Fat 时间戳转 Unix 时间戳
*/
Lib_RTC_UnixType Lib_RTC_Fat2Unix(const Lib_RTC_FatType fat)
{
    Lib_RTC_DateType dt = {0};
    Lib_RTC_Fat2Date(fat, &dt);
    return Lib_RTC_Date2Unix(&dt);
}

/*
 * @brief   Unix 时间戳转 Fat 时间戳
*/
Lib_RTC_FatType Lib_RTC_Unix2Fat(const Lib_RTC_UnixType ts)
{
    Lib_RTC_DateType dt = {0};
    Lib_RTC_Unix2Date(ts, &dt);
    return Lib_RTC_Date2Fat(&dt);
}

/*
 * @brief   Fat 时间戳转日期
*/
void Lib_RTC_Fat2Date(const Lib_RTC_FatType fat, Lib_RTC_DateType* const dt)
{
    uint32_t fdate = (fat >> 16) & 0xFFFF;
    uint32_t ftime = fat & 0xFFFF;

    dt->year   = ((fdate >> 9) & 0x7F) + 1980;
    dt->month  = (fdate >> 5) & 0xF;
    dt->day    =  fdate & 0x1F;
    dt->hour   = (ftime >> 11) & 0x1F;
    dt->minute = (ftime >> 5) & 0x3F;
    dt->second = (ftime & 0x1F) * 2;
}

/*
 * @brief   日期转 Fat 时间戳
*/
Lib_RTC_FatType Lib_RTC_Date2Fat(const Lib_RTC_DateType* const dt)
{
    uint32_t fdate = 0, ftime = 0;

    fdate = (((dt->year - 1980) & 0x7F) << 9) |
            ((dt->month & 0xF) << 5) |
            (dt->day & 0x1F);
    ftime = ((dt->hour & 0x1F) << 11) | 
            ((dt->minute & 0x3F) << 5) |
            ((dt->second / 2) & 0x1F);
    return (fdate << 16) | ftime;
}

/*
 * @brief   若 FatFs 的 FF_FS_NORTC == 0, 则使用 get_fattime() 获得时间戳
 * @return  从 RTC 获得 Unix 时间戳后, 转成 Fat 时间戳
*/
#if (!FF_FS_NORTC)
DWORD get_fattime(void)
{
  Lib_RTC_UnixType ts = Lib_RTC_Read_Time();
  return Lib_RTC_Unix2Fat(ts);
}
#endif