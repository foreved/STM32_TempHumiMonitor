#include "lib_i2c.h"

/*
 * @brief   初始化 I2C
*/
void Lib_I2C_Init(void)
{
    LL_GPIO_InitTypeDef gpio_config = {0};
    LL_I2C_InitTypeDef i2c_config = {0};

    LIB_I2C_PORT_ENCLK();
    LIB_I2C_ENCLK();

    // 配置GPIO
    // I2C_SCL和I2C_SDA都是复用开漏输出
    gpio_config.Pin = LIB_I2C_SCL_PIN;
    gpio_config.Mode = LL_GPIO_MODE_ALTERNATE;
    gpio_config.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
    gpio_config.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    LL_GPIO_Init(LIB_I2C_SCL_PORT, &gpio_config);
    gpio_config.Pin = LIB_I2C_SDA_PIN;
    LL_GPIO_Init(LIB_I2C_SDA_PORT, &gpio_config);

    // 配置I2C
    i2c_config.PeripheralMode = LL_I2C_MODE_I2C;
    i2c_config.DutyCycle = LL_I2C_DUTYCYCLE_2;
    i2c_config.ClockSpeed = LIB_I2C_SPEED;
    i2c_config.OwnAddress1 = LIB_I2C_ADDR;
    i2c_config.OwnAddrSize = LL_I2C_OWNADDRESS1_7BIT;
    i2c_config.TypeAcknowledge = LL_I2C_ACK;
    LL_I2C_Init(LIB_I2C, &i2c_config);
    LL_I2C_Enable(LIB_I2C);
}

// SDA 方向为主机写数据
#define Lib_I2C_Set_Write(addr)     ((addr << 1) | 0)
// SDA 方向为主机读数据
#define Lib_I2C_Set_Read(addr)      ((addr << 1) | 1)

/* 
 * @brief   使用 I2C 向从机发送数据
 * @param   slave_addr 从机地址: bit7～bit1 是 7 位从机地址, bit0 是 0.
 *          buffer 数据缓冲区: 存放发送的数据序列
 *          num 数据个数: 总共发送的数据个数
*/
void Lib_I2C_Send_Data(const uint8_t slave_addr, const uint8_t* const buffer, const uint32_t num)
{
    // 开始通信前, 检查总线是否存在通信事件
    while (LL_I2C_IsActiveFlag_BUSY(LIB_I2C) == SET);
    // 如果总线不在通信, 产生开始信号
    LL_I2C_GenerateStartCondition(LIB_I2C);
    // 等待开始信号发送成功
    while (LL_I2C_IsActiveFlag_SB(LIB_I2C) != SET);
    // 开始信号发送成功, 成为主机, 接着发送从机地址
    // 地址的 LSB 的 bit0 决定读/写 (1/0)
    LL_I2C_TransmitData8(LIB_I2C, Lib_I2C_Set_Write(slave_addr));
    // 等待地址发送完成, 并收到从机的 ACK
    // 主机模式, 收到从机对地址的 ACK, 硬件置位 ADDR
    while (LL_I2C_IsActiveFlag_ADDR(LIB_I2C) != SET);
    // 软件清除 ADDR
    LL_I2C_ClearFlag_ADDR(LIB_I2C);
    // 连续发送数据
    for (uint32_t i = 0; i < num; ++i)
    {
        // 发送前检查数据寄存器是否为空
        while (LL_I2C_IsActiveFlag_TXE(LIB_I2C) != SET);
        LL_I2C_TransmitData8(LIB_I2C, buffer[i]);
    }
    // 确保最后一个字节发送成功
    // 传输模式下, TxE=1 且受到 ACK, 硬件置位 BTF
    while (LL_I2C_IsActiveFlag_BTF(LIB_I2C) != SET);
    // 结束通信
    // 结束信号会硬件清零 TxE 和 BTF
    LL_I2C_GenerateStopCondition(LIB_I2C);
}

/* 
 * @brief   使用 I2C 从从机接收数据
 * @param   slave_addr 从机地址: bit7～bit1 是 7 位从机地址, bit0 是 1.
 *          buffer 数据缓冲区: 存放接收的数据序列
 *          num 数据个数: 总共要接收的数据个数
*/
void Lib_I2C_Receive_Data(const uint8_t slave_addr, uint8_t* const buffer, const uint32_t num)
{
    // 开始通信前, 检查总线是否存在通信事件
    while (LL_I2C_IsActiveFlag_BUSY(LIB_I2C) == SET);
    // 如果总线不在通信, 产生开始信号
    LL_I2C_GenerateStartCondition(LIB_I2C);
    // 等待开始信号发送成功
    while (LL_I2C_IsActiveFlag_SB(LIB_I2C) != SET);
    // 开始信号发送成功, 成为主机, 接着发送从机地址
    // 地址的 LSB 的 bit0 决定读/写 (1/0)
    LL_I2C_TransmitData8(LIB_I2C, Lib_I2C_Set_Read(slave_addr));
    // 等待地址发送完成, 并收到从机的 ACK
    // 主机模式, 收到从机对地址的 ACK, 硬件置位 ADDR
    while (LL_I2C_IsActiveFlag_ADDR(LIB_I2C) != SET);
    // 软件清除 ADDR
    LL_I2C_ClearFlag_ADDR(LIB_I2C);
    
    // 在收到最后一个数据之前, 关闭 ACK 且置位 Stop
    // 这样, 在主机收到最后一个数据之后, 不会回复 ACK (回复 NACK)
    // 接着产生停止信号
    if (num == 1)
    {
        // 仅接收一个字节, 清除 ADDR 后立刻执行
        // 发送 NACK 表示不接受数据了
        LL_I2C_AcknowledgeNextData(LIB_I2C, LL_I2C_NACK);
        // 结束通信
        LL_I2C_GenerateStopCondition(LIB_I2C);
        // 接收一个字节
        while (LL_I2C_IsActiveFlag_RXNE(LIB_I2C) != SET);
        buffer[0] = LL_I2C_ReceiveData8(LIB_I2C);
    }
    else
    {
        for (uint32_t i = 0; i < num; ++i)
        {
            while (LL_I2C_IsActiveFlag_RXNE(LIB_I2C) != SET);
            // 倒数第二个字节已经收到, 正在传输最后一个字节
            // 此时关闭 ACK 并置位 Stop
            if (i == num - 2)
            {
                // 发送 NACK 表示不接受数据了
                LL_I2C_AcknowledgeNextData(LIB_I2C, LL_I2C_NACK);
                // 结束通信
                LL_I2C_GenerateStopCondition(LIB_I2C);
            }
            buffer[i] = LL_I2C_ReceiveData8(LIB_I2C);
        }
    }
}