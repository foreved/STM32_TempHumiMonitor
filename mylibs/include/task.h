#ifndef _TASK_H
#define _TASK_H

#include "ff.h"
#include "lib_usart.h"
#include "lib_i2c.h"
#include "lib_rtc.h"
#include "lib_tool.h"
#include "lib_timer.h"
#include "mod_oled.h"
#include "mod_dht11.h"
#include <string.h>
#include "stm32f1xx_ll_exti.h"

#define LOG_PATH "0:/logs"
#define FILE_PATH "0:/logs/task.csv"

extern uint8_t LOG_SHOW_FLAG;
void Check_FatFs(FATFS *fs);
void Log_Init(void);
void Log_Show(void);
void Log_Apppend(void);
void Upload_Init(void);

#endif