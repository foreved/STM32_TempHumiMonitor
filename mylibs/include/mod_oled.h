#ifndef _MOD_OLED_H
#define _MOD_OLED_H

#include "lib_i2c.h"

// 使用的 OLED 的地址
#define MOD_OLED_ADDR                        0x3C

// 数据序列缓冲区的大小
#define MOD_OLED_BUFFER_SIZE                 100

// 接口
#define MOD_OLED_BUFFER                      Mod_Oled_Buffer
#define MOD_OLED_PBUFFER                     Mod_Oled_PBuffer
// 清空缓冲区
#define Mod_Oled_Buffer_Clear()              (MOD_OLED_PBUFFER = 0)
// 向缓冲区添加数据
#define Mod_Oled_Buffer_Append(data)         (MOD_OLED_BUFFER[MOD_OLED_PBUFFER++] = data)
// 使用 I2C 发送缓冲区数据
#define Mod_Oled_Buffer_Send()               Lib_I2C_Send_Data(MOD_OLED_ADDR, MOD_OLED_BUFFER, MOD_OLED_PBUFFER)
// 使用 I2C 发送自定义缓冲区数据
#define Mod_Oled_Send_Data(pbuffer, num)     Lib_I2C_Send_Data(MOD_OLED_ADDR, (const uint8_t* const)pbuffer, num)

/*
 * @brief   表示光标所在位置
*/
typedef struct 
{
    uint8_t page;     // 页: [0, 7]
    uint8_t column;   // 列: [0, 127]
} Mod_Oled_Pos_Type;


void Mod_Oled_Power_Up(void);
void Mod_Oled_Display_Control(const uint8_t opt);
Mod_Oled_Pos_Type Mod_Oled_Show_String(const Mod_Oled_Pos_Type pos, const char *str);
void Mod_Oled_Fill_Screen(const uint8_t data);

// 清空屏幕
#define Mod_Oled_Clear_Screen()              Mod_Oled_Fill_Screen(0x00)
// 屏幕全亮
#define Mod_Oled_Full_Screen()               Mod_Oled_Fill_Screen(0xFF)

/*
 * @brief   4 种控制, 在 I2C 通信中, 在从机地址之后, 数据/指令之前
 * @note    bit7: 0 表示只有一个控制字节, 之后是若干个指令/数据字节; 1 表示一个控制字节+一个指令/数据字节
 *          bit6: 0 表示后续是指令; 1 表示后续是数据
 *          bit5~0: 无意义, 都是0
*/
#define MOD_OLED_CTRL_ALWAYS_CMD               0x00   // 一个控制 + 若干个指令
#define MOD_OLED_CTRL_ONCE_CMD                 0x80   // 一个控制 + 一个指令
#define MOD_OLED_CTRL_ALWAYS_DATA              0x40   // 一个控制 + 若干个数据
#define MOD_OLED_CTRL_ONCE_DATA                0xC0   // 一个控制 + 一个数据

/*
 * @brief   OLED 显示字符常用的指令
*/
// 显示控制
#define MOD_OLED_CMD_DISPLAY_ON                0xAF   // 开启显示
#define MOD_OLED_CMD_DISPLAY_OFF               0xAE   // 关闭显示
// 设置寻址方式
#define MOD_OLED_CMD_SET_MOD                   0x20   // 设置内存寻址模式
#define MOD_OLED_CMD_MOD_HORIZONTAL            0X00   // 水平寻址 (自动换页)
#define MOD_OLED_CMD_MOD_VERTICAL              0x01   // 垂直寻址 (自动换列)
// 设置地址 (水平或垂直寻址)
#define MOD_OLED_CMD_ADDR_SET_PAGE             0x22   // 设置页地址
#define MOD_OLED_CMD_ADDR_SET_COLUMN           0x21   // 设置列地址

#endif