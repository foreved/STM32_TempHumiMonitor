#include "task.h"

static uint8_t LOG_APPEND_FLAG;
uint8_t LOG_SHOW_FLAG;

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

void Log_Show(void)
{
  FIL file;
  UINT fnum;
  static UINT fp = 0;

  if (LOG_APPEND_FLAG == 0)
  {
    f_open(&file, FILE_PATH, FA_READ);
    f_lseek(&file, fp);
    f_read(&file, Lib_USART_Buffer, LIB_USART_BUFFER_MAXSIZE-1, &fnum);
    Lib_USART_Buffer[fnum] = '\0';
    fp += fnum;
    if (f_eof(&file))
    {
      fp = 0;
      LOG_SHOW_FLAG = RESET;
    }
    f_close(&file);

    LL_DMA_DisableChannel(LIB_USART_DMA, LIB_USART_DMA_CH);
    LL_DMA_SetDataLength(LIB_USART_DMA, LIB_USART_DMA_CH, fnum);
    LL_DMA_EnableChannel(LIB_USART_DMA, LIB_USART_DMA_CH);
  }
}

void Log_Apppend(void)
{
  FIL file;
  FRESULT fres;
  UINT fnum;
  const char* buffer;

  // 正在更新日志
  LOG_APPEND_FLAG = 1;
  (void)fres;
  fres = f_open(&file, FILE_PATH, FA_OPEN_EXISTING | FA_WRITE);
  fres = f_lseek(&file, f_size(&file));
  buffer = Lib_Tool_Format_String("%x,%x,%x\n", Real_Time_TempHumi.timestamp, Real_Time_TempHumi.temp, Real_Time_TempHumi.humi);
  fres = f_write(&file, buffer, strlen(buffer), &fnum);
  f_close(&file);
  // 更新结束
  LOG_APPEND_FLAG = 0;
}

void Upload_Init(void)
{
  LL_GPIO_InitTypeDef gpio_config = {0};
  LL_EXTI_InitTypeDef exti_config = {0};

  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOC);

  LL_GPIO_AF_SetEXTISource(LL_GPIO_AF_EXTI_PORTC, LL_GPIO_AF_EXTI_LINE13);
  gpio_config.Pin = LL_GPIO_PIN_13;
  gpio_config.Mode = LL_GPIO_MODE_INPUT;
  gpio_config.Pull = LL_GPIO_PULL_DOWN;
  LL_GPIO_Init(GPIOC, &gpio_config);

  exti_config.Line_0_31 = LL_EXTI_LINE_13;
  exti_config.Mode = LL_EXTI_MODE_IT;
  exti_config.Trigger = LL_EXTI_TRIGGER_RISING;
  exti_config.LineCommand = ENABLE;
  LL_EXTI_Init(&exti_config);

  NVIC_SetPriority(EXTI15_10_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 5, 0));
  NVIC_EnableIRQ(EXTI15_10_IRQn);
}


void EXTI15_10_IRQHandler(void)
{
  if (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_13) == SET)
  {
    LOG_SHOW_FLAG = 1;
    LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_13);
  }
}