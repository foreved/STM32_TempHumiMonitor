#include "task.h"

void Check_FatFs(FATFS* fs)
{
  FRESULT fres;

  fres = f_mount(fs, "0:", 1);
  if (fres == FR_NO_FILESYSTEM)
  {
    BYTE work_buffer[FF_MAX_SS];
    fres = f_mkfs("0:", (void*)0, work_buffer, FF_MAX_SS);
    if (fres != FR_OK)
    {
      Lib_USART_Send_String(Lib_Tool_Format_String("Error: FatFs's FRESULT is %d. Stop...\n", fres));
      while (1);
    }
    fres = f_mount(fs, "0:", 1);
  }
  if (fres != FR_OK)
  {
    Lib_USART_Send_String(Lib_Tool_Format_String("Error: FatFs's FRESULT is %d. Stop...\n", fres));
    while (1);
  }
}

void Log_Init(void)
{
  FRESULT fres;
  char* buffer;
  FIL file;
  UINT fnum;

  fres = f_stat(LOG_PATH, (void*)0);
  if (fres == FR_NO_FILE)
  {
    fres = f_mkdir(LOG_PATH);
  }
  if (fres != FR_OK)
  {
    Lib_USART_Send_String(Lib_Tool_Format_String("Error: FatFs's FRESULT is %d. Stop...\n", fres));
    while (1);
  }

  fres = f_stat(FILE_PATH, (void*)0);
  if (fres == FR_NO_FILE)
  {
    fres = f_open(&file, FILE_PATH, FA_CREATE_NEW | FA_WRITE);
    buffer = "TimeStamp,Temperature,Humidity\n";
    fres = f_write(&file, buffer, strlen(buffer), &fnum);
  }
  if (fres != FR_OK)
  {
    Lib_USART_Send_String(Lib_Tool_Format_String("Error: FatFs's FRESULT is %d. Stop...\n", fres));
    while (1);
  }
  f_close(&file);
}

void Log_Show(uint8_t num)
{
  FIL file;
  char buffer[100];

  f_open(&file, FILE_PATH, FA_READ);
  for (uint8_t i = 0; i < num; ++i)
  {
    if (f_gets(buffer, 100, &file) == (void*)0) break;
    Lib_USART_Send_String(buffer);
  }
}

void Log_Apppend(void)
{
  FIL file;
  FRESULT fres;
  UINT fnum;
  const char* buffer;

  (void)fres;
  fres = f_open(&file, FILE_PATH, FA_OPEN_EXISTING | FA_WRITE);
  fres = f_lseek(&file, f_size(&file));
  buffer = Lib_Tool_Format_String("%x,%x,%x\n", Real_Time_TempHumi.timestamp, Real_Time_TempHumi.temp, Real_Time_TempHumi.humi);
  fres = f_write(&file, buffer, strlen(buffer), &fnum);
  f_close(&file);
}