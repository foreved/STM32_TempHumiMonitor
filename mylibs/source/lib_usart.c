#include "lib_usart.h"
#include <stdarg.h>

uint8_t Lib_USART_Buffer[LIB_USART_BUFFER_MAXSIZE];

void Lib_USART_Init(void)
{
  LL_GPIO_InitTypeDef gpio_config = {0};
  LL_USART_InitTypeDef usart_config = {0};
  #if LIB_USART_DMA_EN
    LL_DMA_InitTypeDef dma_config = {0};
  #endif

  // 开启外设的时钟
  LIB_USART_ENCLK();
  LIB_USART_PORT_ENCLK();

  // TX为复用推挽输出
  gpio_config.Pin = LIB_USART_TX_PIN;
  gpio_config.Mode = LL_GPIO_MODE_ALTERNATE;
  gpio_config.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  gpio_config.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  LL_GPIO_Init(LIB_USART_TX_PORT, &gpio_config);
  // RX为浮空输入
  gpio_config.Pin = LIB_USART_RX_PIN;
  gpio_config.Mode = LL_GPIO_MODE_FLOATING;
  gpio_config.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  LL_GPIO_Init(LIB_USART_RX_PORT, &gpio_config);

  // 数据帧格式：1位起始位+8位数据+无校验+1位停止位
  usart_config.DataWidth = LL_USART_DATAWIDTH_8B;
  usart_config.Parity = LL_USART_PARITY_NONE;
  usart_config.StopBits = LL_USART_STOPBITS_1;
  // 波特率由宏USART_BAUD_RATE定义
  usart_config.BaudRate = LIB_USART_BAUD_RATE;
  // 工作模式为全双工
  usart_config.TransferDirection = LL_USART_DIRECTION_TX_RX;
  // 不使用硬件控制流，即RTS/CTS
  usart_config.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
  // 初始化USART
  LL_USART_Init(LIB_USART, &usart_config);

  // 配置中断
  #if LIB_USART_IT_EN
    NVIC_SetPriority(LIB_USART_IRQ, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 
                    LIB_USART_PREEMPT_PRIORITY, LIB_USART_SUB_PRIORITY));
    NVIC_EnableIRQ(LIB_USART_IRQ);

    // 接收中断
    #if LIB_USART_IT_RX_EN
      LL_USART_EnableIT_RXNE(LIB_USART);
    #endif
  #endif

  // 配置DMA
  #if LIB_USART_DMA_EN
    LIB_USART_DMA_ENCLK();

    dma_config.Priority = LIB_USART_DMA_PRIORITY;
    dma_config.Direction = LIB_USART_DMA_DIRECTION;
    dma_config.Mode = LIB_USART_DMA_MODE;
    dma_config.PeriphOrM2MSrcAddress = LIB_USART_DMA_PADDR;
    dma_config.MemoryOrM2MDstAddress = LIB_USART_DMA_MADDR;
    dma_config.PeriphOrM2MSrcIncMode = LIB_USART_DMA_PINC;
    dma_config.MemoryOrM2MDstIncMode = LIB_USART_DMA_MINC;
    dma_config.PeriphOrM2MSrcDataSize = LIB_USART_DMA_PDSIZE;
    dma_config.MemoryOrM2MDstDataSize = LIB_USART_DMA_MDSIZE;
    dma_config.NbData = LIB_USART_DMA_NDATA;
    LL_DMA_Init(LIB_USART_DMA, LIB_USART_DMA_CH, &dma_config);
    // LIB_USART_DMA_CH_EN();
    // 允许USART_TX发起DMA请求
    LL_USART_EnableDMAReq_TX(LIB_USART);
  #endif

  // 使能USART
  LL_USART_Enable(LIB_USART);
}

// 发送一个字节
void Lib_USART_Send_Byte(const uint8_t data)
{
  // 等待TXE被硬件置位，表示TDR空，可以写入数据
  while (LL_USART_IsActiveFlag_TXE(LIB_USART) != SET);
  // 向TDR写入数据
  // 读取USART_SR+写入USART_DR，会清除TXE和TC
  LL_USART_TransmitData8(LIB_USART, data);
  // 等待TC被硬件置位，表示发送完成
  while (LL_USART_IsActiveFlag_TC(LIB_USART) != SET);
}

// 发送字符串
void Lib_USART_Send_String(const char * str)
{
  uint8_t i = 0;

  while (str[i] != '\0')
  {
    // 等待TDR空
    while (LL_USART_IsActiveFlag_TXE(LIB_USART) != SET);
    // 写入字符
    LL_USART_TransmitData8(LIB_USART, str[i]);
    ++i;
  }
  // 等待传输完成
  while (LL_USART_IsActiveFlag_TC(LIB_USART) != SET);
}

// array为要倒转的序列, st为开始下标, ed为结束下标
static void Lib_USART_Reverse_String(uint8_t * const array, const uint8_t st, const uint8_t ed)
{
  uint8_t p1 = st, p2 = ed, tmp = 0;

  while (p1 < p2)
  {
    tmp = array[p1];
    array[p1] = array[p2];
    array[p2] = tmp;
    ++p1;
    --p2;
  }
}

void Lib_USART_Int2Char_DEC(const int num, uint8_t * const buffer)
{
  uint8_t p = 0, st = 0; // p遍历buffer; st记录反转的开始下标
  int tmp = num;

  if (num < 0)
  {
    tmp = -tmp;
    st = 1;
    buffer[p] = '-';
    ++p;
  }
  else if (num == 0)
  {
    buffer[0] = '0';
    buffer[1] = '\0';
    return;
  }
  while (tmp > 0)
  {
    buffer[p] = tmp % 10 + '0';
    ++p;
    tmp /= 10;
  }
  Lib_USART_Reverse_String(buffer, st, p - 1);
  buffer[p] = '\0';
}

void Lib_USART_Int2Char_HEX(const int num, uint8_t * const buffer)
{
  uint8_t p = 0, st = 0, v = 0; // p遍历buffer; st记录反转的开始下标; v为取余的结果
  int tmp = num;

  buffer[0] = '0';
  buffer[1] = 'x';
  p = 2;
  if (num < 0)
  {
    tmp = -tmp;
    buffer[p] = '-';
    ++p;
  }
  else if (num == 0)
  {
    buffer[2] = '0';
    buffer[3] = '\0';
    return;
  }
  st = p;
  while (tmp > 0)
  {
    v = tmp % 16;
    if (v < 10)
    {
      buffer[p] = v + '0';
    }
    else
    {
      buffer[p] = v - 10 + 'A';
    }
    ++p;
    tmp /= 16;
  }
  Lib_USART_Reverse_String(buffer, st, p - 1);
  buffer[p] = '\0';
}

// num_frac_bits指定小数位数，只是截断显示
void Lib_USART_Double2Char(const double num, uint8_t * const buffer, const uint8_t num_frac_bits)
{
  uint8_t p = 0, st = 0;
  int int_part = 0;            // 存num的整数部分
  double frac_part = 0;        // 存num的小数部分

  if (num < 0.0)
  {
    buffer[p] = '-';
    ++p;
    int_part = (uint32_t)(-num);
    frac_part = (-num) - int_part;
  }
  else
  {
    int_part = (uint32_t)num;
    frac_part = num - int_part;
  }
  // 处理整数部分
  if (int_part == 0)
  {
    buffer[p] = '0';
    ++p;
  }
  else
  {
    st = p;
    while (int_part > 0)
    {
      buffer[p] = int_part % 10 + '0';
      ++p;
      int_part /= 10;
    }
    Lib_USART_Reverse_String(buffer, st, p - 1);
  }
  buffer[p] = '.';
  ++p;
  // 处理小数部分
  if (frac_part == 0.0)
  {
    buffer[p] = '0';
    ++p;
  }
  else
  {
    // num_frac_bits显示小数位数
    for (uint8_t i = 0; i < num_frac_bits + 1; ++i)
    {
      frac_part *= 10;
      buffer[p] = (uint8_t)frac_part + '0';
      ++p;
      frac_part = frac_part - (uint8_t)frac_part;
    }
    if (buffer[p - 1] > '4') buffer[p - 2] += 1;
    --p;
  }
  buffer[p] = '\0';
}

// 发送格式化字符串
// %d: 十进制整型; %x: 十六进制整型; %f: 浮点数(double); 
void Lib_USART_Send_fString(const char * str, ...)
{
  uint8_t i = 0;          // 遍历字符串

  int arg_int = 0;          // 整型参数
  double arg_double = 0.0;  // 浮点型
  char* arg_str = (void*)0;     // 字符串
  uint8_t buffer[20] = {0}; // 缓冲区
  
  va_list ap;           // 声明ap容纳不定参数
  va_start(ap, str);    // 初始化ap

  while (str[i] != '\0')
  {
    // 非格式化字符
    if (str[i] != '%')
    {
      while (LL_USART_IsActiveFlag_TXE(LIB_USART) != SET);
      LL_USART_TransmitData8(LIB_USART, str[i]);
      ++i;
    }
    else // 格式化字符
    {
      ++i;
      switch (str[i])
      {
        // 十进制整型
        case 'd':
          arg_int = va_arg(ap, int);
          Lib_USART_Int2Char_DEC(arg_int, buffer);
          ++i;
          break;
        // 十六进制整型
        case 'x':
          arg_int = va_arg(ap, int);
          Lib_USART_Int2Char_HEX(arg_int, buffer);
          ++i;
          break;
        // 浮点数
        case 'f':
          arg_double = va_arg(ap, double);
          // 默认打印3位小数
          Lib_USART_Double2Char(arg_double, buffer, 3);
          ++i;
          break;
        // 字符串
        case 's':
          arg_str = va_arg(ap, char*);
          Lib_USART_Send_String(arg_str);
          ++i;
          continue;
        default:
          // 报错，待实现
          break;
      }
      // 发送buffer数据
      for (uint8_t i = 0; buffer[i] != '\0'; ++i)
      {
        while (LL_USART_IsActiveFlag_TXE(LIB_USART) != SET);
        LL_USART_TransmitData8(LIB_USART, buffer[i]);
      }
    }
  }
  // 确保发送完成
  while (LL_USART_IsActiveFlag_TC(LIB_USART) != SET);

  va_end(ap);           // 释放ap
}

#if LIB_USART_IT_EN
  void Lib_USART_IT_Handler(void)
  {
    uint8_t tmp = 0;
    if (LL_USART_IsActiveFlag_RXNE(LIB_USART) == SET)
    {
      tmp = LL_USART_ReceiveData8(LIB_USART);
      Lib_USART_Send_fString("RX IT: %x\n", tmp);
    }
  }
#endif