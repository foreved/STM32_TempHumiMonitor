#include <stdarg.h>
#include <string.h>
#include "lib_tool.h"

/*
 * @brief   不同位表示相应外设是否开启. 0: 关闭, 1: 开启
 * @note    bit0: SysTick
 *          bit1: DWT
 */
static uint8_t LIB_TOOL_STATUS;
#define LIB_TOOL_STATUS_SYSTICK_Pos 0
#define LIB_TOOL_STATUS_SYSTICK_Mask (1 << LIB_TOOL_STATUS_SYSTICK_Pos)
#define LIB_TOOL_STATUS_DWT_Pos 1
#define LIB_TOOL_STATUS_DWT_Mask (1 << LIB_TOOL_STATUS_DWT_Pos)

/*
 * @brief   配置并开启 SysTick, 时基为 1ms
 * @param   无
 * @return  无
 * @note    SysTick使用 AHB 作为时钟源 (不是 AHB/8), 使用前必须配置
 *          lib_tool.h 中的 LIB_TOOL_AHB_FREQUENCY, 即 AHB 时钟频率.
 */
void Lib_Tool_SysTick_Init(void)
{
    SysTick->LOAD = LIB_TOOL_AHB_FREQUENCY / 1000 - 1; // 1 ms
    SysTick->VAL = 0;
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_CLKSOURCE_Msk; // 使用 AHB, 并开启 SysTick
}

/*
 * @brief   使用 SysTick 实现毫秒级延时
 * @param   num_ms: 延时 num_ms 毫妙
 * @return  无
 * @note    1) SysTick使用 AHB 作为时钟源 (不是 AHB/8), 使用前必须配置
 *          lib_tool.h 中的 LIB_TOOL_AHB_FREQUENCY, 即 AHB 时钟频率.
 *          2) 推荐场景: 非高精度, 短期, 毫秒级延时
 */
void Lib_Tool_SysTick_Delay_ms(const uint16_t num_ms)
{
    SysTick->VAL = 0;
    for (uint16_t i = 0; i < num_ms; ++i)
        while ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) == 0)
            ;
}

/*
 * @brief   开启 DWT, 时基时 1/LIB_TOOL_AHB_FREQUENCY
 * @param   无
 * @return  无
 */
void Lib_Tool_DWT_Init(void)
{
    // 使能 DWT
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    // 清零计数器
    DWT->CYCCNT = 0;
    // 开启计数器
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

    // DWT 已开启
    LIB_TOOL_STATUS |= LIB_TOOL_STATUS_DWT_Mask;
}

/*
 * @brief   使用 DWT 实现微秒级延时
 * @param   num_us: 延时 num_us 毫妙
 * @return  无
 * @note    1) 使用前要正确配置 DWT 的时钟源频率, lib_tool.h 中的 LIB_TOOL_AHB_FREQUENCY.
 *          2) 使用前必须已经开启 DWT
 *          2) 推荐场景: 高精度, 短期, 微秒级延时
 */
void Lib_Tool_DWT_Delay_us(const uint16_t num_us)
{
    // 不清零也可以, 因为是无符号数
    // e.g. 0x00000003−0xFFFFFFFE=0x00000005
    uint32_t start = DWT->CYCCNT;
    // num_us 对应的 DWT 计数器的计时数
    uint32_t ticks = LIB_TOOL_AHB_FREQUENCY / 1000000 * num_us;
    while ((DWT->CYCCNT - start) < ticks)
        ;
}

/*
 * @brief   使用 DWT 实现计时器, 结束计时, 与 Lib_Tool_DWT_Timer_Start() 一起使用. 微秒级计时 (is_us=1),
 *          毫秒级计时 (is_us=0).
 * @param   start 开始计时时刻
 *          is_us 模式选择:
 *              -0: 毫秒级
 *              -1: 微秒级
 * @return  从 start 到此刻的时间间隔, 毫秒或微秒为单位
 * @note    1) 使用前要 Lib_Tool_DWT_Timer_Start() 获取开始时刻
 *          2) 推荐场景: 高精度, 微妙级或毫秒级, 短期计时
 */
uint32_t Lib_Tool_DWT_Timer_End(const uint32_t start, const uint8_t is_us)
{
    uint32_t ticks = DWT->CYCCNT - start;
    if (is_us) // 毫秒级
    {
        return (uint32_t)((uint64_t)ticks * 1000000 / LIB_TOOL_AHB_FREQUENCY);
    }
    else // 微秒级
    {
        return (uint32_t)((uint64_t)ticks * 1000 / LIB_TOOL_AHB_FREQUENCY);
    }
}

static char LIB_TOOL_FORMAT_BUFFER[200];
static uint8_t LIB_TOOL_FORMAT_PBUFFER;

const char* const Lib_Tool_Format_String(const char *str, ...)
{
    int arg_int;       // 整型参数
    float arg_float;   // 浮点型
    char *arg_str;     // 字符串

    va_list ap;        // 声明ap容纳不定参数
    va_start(ap, str); // 初始化ap

    LIB_TOOL_FORMAT_PBUFFER = 0;
    const char *begin = str, *end;
    while (1)
    {
        end = strchr(begin, '%');
        if (end == NULL)
        {
            memcpy(LIB_TOOL_FORMAT_BUFFER + LIB_TOOL_FORMAT_PBUFFER, begin, strlen(begin));
            LIB_TOOL_FORMAT_PBUFFER += strlen(begin);
            break;
        }

        // 非格式化字符
        memcpy(LIB_TOOL_FORMAT_BUFFER + LIB_TOOL_FORMAT_PBUFFER, begin, end - begin);
        LIB_TOOL_FORMAT_PBUFFER += end - begin;

        // 格式化字符
        switch (*(end + 1))
        {
        case 'd':
            arg_int = va_arg(ap, int);
            LIB_TOOL_FORMAT_PBUFFER += Lib_Tool_Int2Char_DEC(arg_int, LIB_TOOL_FORMAT_BUFFER + LIB_TOOL_FORMAT_PBUFFER);
            begin = end + 2;
            break;
        case 'x':
            arg_int = va_arg(ap, int);
            LIB_TOOL_FORMAT_PBUFFER += Lib_Tool_Int2Char_HEX(arg_int, LIB_TOOL_FORMAT_BUFFER + LIB_TOOL_FORMAT_PBUFFER);
            begin = end + 2;
            break;
        case '.':
            arg_float = (float)va_arg(ap, double);
            LIB_TOOL_FORMAT_PBUFFER += Lib_Tool_Float2Char(arg_float, LIB_TOOL_FORMAT_BUFFER + LIB_TOOL_FORMAT_PBUFFER, *(end + 2) - '0');
            begin = end + 4;
            break;
        case 's':
            arg_str = va_arg(ap, char*);
            memcpy(LIB_TOOL_FORMAT_BUFFER + LIB_TOOL_FORMAT_PBUFFER, arg_str, strlen(arg_str));
            LIB_TOOL_FORMAT_PBUFFER += strlen(arg_str);
            begin = end + 2;
            break;
        case '%':
            LIB_TOOL_FORMAT_BUFFER[LIB_TOOL_FORMAT_PBUFFER++] = '%';
            begin = end + 2;
            break;
        default:
            break;
        }
    }
    va_end(ap); // 释放ap
    LIB_TOOL_FORMAT_BUFFER[LIB_TOOL_FORMAT_PBUFFER] = '\0';
    return LIB_TOOL_FORMAT_BUFFER;
}

static void Lib_Tool_Reverse_String(char* begin, char* end)
{
    uint8_t tmp;

    while (begin < end)
    {
        tmp = *begin;
        *begin = *end;
        *end = tmp;
        ++begin;
        --end;
    }
}

const uint8_t Lib_Tool_Int2Char_DEC(const int num, char *buffer)
{
    uint8_t p = 0, begin = 0;
    int tmp = num;

    if (num < 0)
    {
        tmp = -tmp;
        buffer[p++] = '-';
        begin = 1;
    }
    else if (num == 0)
    {
        buffer[p++] = '0';
    }
    while (tmp > 0)
    {
        buffer[p++] = tmp % 10 + '0';
        tmp /= 10;
    }
    Lib_Tool_Reverse_String(buffer + begin, buffer + p - 1);
    buffer[p] = '\0';
    return p;
}

const uint8_t Lib_Tool_Int2Char_HEX(const int num, char *buffer)
{
    uint8_t p = 0, begin = 0, v = 0;
    int tmp = num;

    if (num < 0)
    {
        tmp = -tmp;
        buffer[p++] = '-';
        begin = 1;
    }
    else if (num == 0)
    {
        buffer[p++] = '0';
    }
    while (tmp > 0)
    {
        v = tmp % 16;
        if (v < 10)
        {
            buffer[p++] = v + '0';
        }
        else
        {
            buffer[p++] = v - 10 + 'A';
        }
        tmp /= 16;
    }
    Lib_Tool_Reverse_String(buffer + begin, buffer + p - 1);
    buffer[p] = '\0';
    return p;
}

const uint8_t Lib_Tool_Float2Char(const float num, char *buffer, const uint8_t num_frac_bits)
{
    uint8_t p = 0;
    int int_part;     // 存num的整数部分
    double frac_part; // 存num的小数部分

    int_part = (int32_t)num;
    frac_part = num < 0 ? int_part - num : num - int_part;

    // 整数部分
    if (int_part == 0 && num < 0)
    {
        buffer[p++] = '-';
    }
    p += Lib_Tool_Int2Char_DEC(int_part, buffer + p);

    buffer[p++] = '.';

    // 处理小数部分
    for (uint8_t i = 0; i < num_frac_bits + 1; ++i)
    {
        frac_part *= 10;
        buffer[p++] = (uint8_t)frac_part + '0';
        frac_part = frac_part - (uint8_t)frac_part;
    }
    if (num_frac_bits == 0)
    {
        buffer[p - 3] += buffer[p - 1] > '4' ? 1 : 0;
        p -= 2;
        buffer[p] = '\0';
    }
    else
    {
        buffer[p - 2] += buffer[p - 1] > '4' ? 1 : 0;
        buffer[--p] = '\0';
    }
    
    return p;
}
