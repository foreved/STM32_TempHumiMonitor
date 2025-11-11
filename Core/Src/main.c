/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "task.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define LOG_PATH   "0:/logs"
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_AFIO);
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);

  /* System interrupt init*/
  NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

  /* SysTick_IRQn interrupt configuration */
  NVIC_SetPriority(SysTick_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),15, 0));

  /** NOJTAG: JTAG-DP Disabled and SW-DP Enabled
  */
  LL_GPIO_AF_Remap_SWJ_NOJTAG();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();

  Lib_RTC_DateType date;
  Mod_Oled_Pos_Type pos = {0, 0};
  FATFS fs;

  // SysTick 和 DWT
  Lib_Tool_SysTick_Init(); // 毫秒延时
  Lib_Tool_DWT_Init();     // 微妙计时
  // RTC
  Lib_RTC_Init();
  // USART
  Lib_USART_Init();
  // OLED
  Lib_I2C_Init();
  Mod_Oled_Power_Up();
  // DHT11
  Mod_DHT11_GPIO_Init();
  // FatFs
  Check_FatFs(&fs);
  f_unlink(FILE_PATH);
  Log_Init();
  // Timer
  Lib_Timer_Init();
  
  LL_TIM_SetCounter(LIB_TIMER, 0);
  LL_TIM_EnableCounter(LIB_TIMER);
  while (1)
  {
    if (LIB_TIMER_CYCLE_FLAG == SET)
    {
      // 连续读取两次数据, 获得此时的温湿度数据
      Real_Time_TempHumi = Mod_DHT11_Once_Com();
      Lib_Tool_SysTick_Delay_ms(100); // 间隔 100ms, 采集数据
      Real_Time_TempHumi = Mod_DHT11_Once_Com();

      // 获取时间
      Lib_RTC_Unix2Date(Real_Time_TempHumi.timestamp, &date);
      // 上位机显示
      // Lib_USART_Send_String(Lib_Tool_Format_String("%d-%d-%d, %d:%d:%d\n", date.year, date.month, date.day, date.hour, date.minute, date.second));
      // Lib_USART_Send_String(Lib_Tool_Format_String("Temperature: %d.%d\n", Real_Time_TempHumi.temp / 10, Lib_Tool_Abs(Real_Time_TempHumi.temp % 10)));
      // Lib_USART_Send_String(Lib_Tool_Format_String("Humidity: %d.%d\n\n", Real_Time_TempHumi.humi / 10, Real_Time_TempHumi.humi % 10));
      
      // OLED 显示
      Mod_Oled_Clear_Screen();
      pos = (Mod_Oled_Pos_Type){0, 0};
      pos = Mod_Oled_Show_String(pos, Lib_Tool_Format_String("%d-%d-%d", date.year, date.month, date.day));
      pos = (Mod_Oled_Pos_Type){pos.page += 2, 0};
      pos = Mod_Oled_Show_String(pos, Lib_Tool_Format_String("%d:%d:%d", date.hour, date.minute, date.second));
      pos = (Mod_Oled_Pos_Type){pos.page += 2, 0};
      pos = Mod_Oled_Show_String(pos, Lib_Tool_Format_String("Temp: %d.%d deg", Real_Time_TempHumi.temp / 10, Lib_Tool_Abs(Real_Time_TempHumi.temp % 10)));
      pos = (Mod_Oled_Pos_Type){pos.page += 2, 0};
      pos = Mod_Oled_Show_String(pos, Lib_Tool_Format_String("Humi: %d.%d%%", Real_Time_TempHumi.humi / 10, Real_Time_TempHumi.humi % 10));

      Log_Apppend();

      LIB_TIMER_CYCLE_FLAG = 0;
    }
  }
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_2);
  while(LL_FLASH_GetLatency()!= LL_FLASH_LATENCY_2)
  {
  }
  LL_RCC_HSE_Enable();

   /* Wait till HSE is ready */
  while(LL_RCC_HSE_IsReady() != 1)
  {

  }
  LL_PWR_EnableBkUpAccess();
  if(LL_RCC_GetRTCClockSource() != LL_RCC_RTC_CLKSOURCE_LSE)
  {
    LL_RCC_ForceBackupDomainReset();
    LL_RCC_ReleaseBackupDomainReset();
  }
  LL_RCC_LSE_Enable();

   /* Wait till LSE is ready */
  while(LL_RCC_LSE_IsReady() != 1)
  {

  }
  if(LL_RCC_GetRTCClockSource() != LL_RCC_RTC_CLKSOURCE_LSE)
  {
    LL_RCC_SetRTCClockSource(LL_RCC_RTC_CLKSOURCE_LSE);
  }
  LL_RCC_EnableRTC();
  LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSE_DIV_1, LL_RCC_PLL_MUL_9);
  LL_RCC_PLL_Enable();

   /* Wait till PLL is ready */
  while(LL_RCC_PLL_IsReady() != 1)
  {

  }
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_2);
  LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);

   /* Wait till System clock is ready */
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
  {

  }
  LL_Init1msTick(72000000);
  LL_SetSystemCoreClock(72000000);
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOC);
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOD);
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOA);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
