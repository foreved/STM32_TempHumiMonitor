#include "lib_spi.h"

void Lib_SPI_Init(void)
{
    LL_GPIO_InitTypeDef gpio_config = {0};
    LL_SPI_InitTypeDef spi_config = {0};

    LIB_SPI_PORT_ENCLK();
    LIB_SPI_ENCLK();

    // 配置 GPIO
    // SPI_NSS 采用推挽输出 (软件控制; 如果是硬件控制, 需要复用)
    // SPI_SCK, SPI_MOSI 采用复用推挽输出
    // SPI_MISO 采用浮空输入
    gpio_config.Pin = LIB_SPI_NSS_PIN;
    gpio_config.Mode = LL_GPIO_MODE_OUTPUT;  // SPI_NSS 采用软件控制, 不需要复用
    gpio_config.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    gpio_config.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    LL_GPIO_Init(LIB_SPI_NSS_PORT, &gpio_config);
    gpio_config.Pin = LIB_SPI_MOSI_PIN;
    gpio_config.Mode = LL_GPIO_MODE_ALTERNATE;
    LL_GPIO_Init(LIB_SPI_MOSI_PORT, &gpio_config);
    gpio_config.Pin = LIB_SPI_SCK_PIN;
    LL_GPIO_Init(LIB_SPI_SCK_PORT, &gpio_config);
    gpio_config.Pin = LIB_SPI_MISO_PIN;
    gpio_config.Mode = LL_GPIO_MODE_FLOATING;
    LL_GPIO_Init(LIB_SPI_MISO_PORT, &gpio_config);

    // 配置 SPI
    spi_config.TransferDirection = LL_SPI_FULL_DUPLEX;
    spi_config.Mode = LL_SPI_MODE_MASTER;
    spi_config.DataWidth = LL_SPI_DATAWIDTH_8BIT;
    spi_config.ClockPolarity = LIB_SPI_CPOL;
    spi_config.ClockPhase = LIB_SPI_CPHA;
    spi_config.NSS = LL_SPI_NSS_SOFT;
    spi_config.BaudRate = LIB_SPI_BAUD_RATE;
    spi_config.BitOrder = LIB_SPI_BIT_ORDER;
    spi_config.CRCCalculation = LL_SPI_CRCCALCULATION_DISABLE;
    LL_SPI_Init(LIB_SPI, &spi_config);

    // NSS 置 1, 再使能 SPI
    LIB_SPI_STOP();
    LL_SPI_Enable(LIB_SPI);
}

// 使用前需要 LIB_SPI_START()
uint8_t Lib_SPI_Send_Byte(uint8_t data)
{
    while (LL_SPI_IsActiveFlag_TXE(LIB_SPI) != SET);
    LL_SPI_TransmitData8(LIB_SPI, data);
    // SPI 全双工工作, 发送的同时也在接收
    // 发送了一个数据, 也意味着接收了一个数据
    // 接收的数据是否有效取决于实际情况
    while (LL_SPI_IsActiveFlag_RXNE(LIB_SPI) != SET);
    return LL_SPI_ReceiveData8(LIB_SPI);
}

// 使用前需要 LIB_SPI_START()
uint8_t Lib_SPI_Receive_Byte(void)
{
    // SPI 全双工工作, 发送的同时也在接收
    // 接收数据, 可以发送一个任意数据
    return Lib_SPI_Send_Byte(LIB_SPI_DUMMY);
}