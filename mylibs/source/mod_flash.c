#include "mod_flash.h"
#include "lib_usart.h"
#include "lib_tool.h"

static void Mod_Flash_Wait_Busy();
static void Mod_Flash_Write_Enable(void);
static void Mod_Flash_Send_Addr(const uint32_t addr);
static ErrorStatus Mod_Flash_Write_Page(const uint8_t *const pbuffer, const uint32_t addr, const uint16_t num_write);

// 读取 JEDCE_ID
uint32_t Mod_Flash_Read_JEDCE_ID(void)
{
    uint32_t manufacturer = 0, memory_type = 0, capability = 0;
    // 开始通信
    Mod_Flash_COM_Start();

    Mod_Flash_Send_Byte(MOD_FLASH_W25Q64_JEDEC_ID);
    manufacturer = Mod_Flash_Receive_Byte();
    memory_type = Mod_Flash_Receive_Byte();
    capability = Mod_Flash_Receive_Byte();

    // 结束通信
    Mod_Flash_COM_Stop();

    return (manufacturer << 16) | (memory_type << 8) | capability;
}

// 等待 FLASH 忙碌
static void Mod_Flash_Wait_Busy()
{
    uint8_t status = 0;
    Mod_Flash_COM_Start();

    Mod_Flash_Send_Byte(MOD_FLASH_W25Q64_READ_STATUS_REGISTER_1);
    do // 发送读取状态寄存器后, FLASH 会一直返回状态, 直到通信结束
    {
        status = Mod_Flash_Receive_Byte();
    } while (((status & MOD_FLASH_BUSY_Msk) >> MOD_FLASH_BUSY_Pos) == MOD_FLASH_BUSY);

    Mod_Flash_COM_Stop();
}

// 写使能
static void Mod_Flash_Write_Enable(void)
{
    Mod_Flash_COM_Start();
    Mod_Flash_Send_Byte(MOD_FLASH_W25Q64_WRITE_ENABLE);
    Mod_Flash_COM_Stop();
}

// 发送地址
// 使用前必须要开启通信
static void Mod_Flash_Send_Addr(const uint32_t addr)
{
    // MSB 在前
    Mod_Flash_Send_Byte((addr & 0xFF0000) >> 16);
    Mod_Flash_Send_Byte((addr & 0x00FF00) >> 8);
    Mod_Flash_Send_Byte(addr & 0x0000FF);
}

// 擦除扇区
// addr 必须对齐扇区大小
ErrorStatus Mod_Flash_Erase_Sector(const uint32_t addr)
{
    if (addr % MOD_FLASH_SECTOR_SIZE != 0)
    {
        return ERROR; // 扇区擦除必须对齐
    }

    Mod_Flash_Write_Enable();
    Mod_Flash_Wait_Busy();
    Mod_Flash_COM_Start();
    Mod_Flash_Send_Byte(MOD_FLASH_W25Q64_SECTOR_ERASE_4KB);
    // FLASH 有 24 位地址, 需对齐 4KB, MSB 在前
    Mod_Flash_Send_Addr(addr);
    Mod_Flash_COM_Stop();
    Mod_Flash_Wait_Busy();
    return SUCCESS;
}

// 若 addr 与页大小对齐, num_write 要不大于页大小
// 若 addr 不与页大小对齐, num_write 要不大于 addr 所在页的部分页大小
// 写入不能跨页, 只能在 addr 所在页写入
// 写入操作一般不调用擦除函数, 因为 Flash 擦除次数有限, 应避免频繁擦除
static ErrorStatus Mod_Flash_Write_Page(const uint8_t *const pbuffer, const uint32_t addr, const uint16_t num_write)
{
    uint32_t num_remain = MOD_FLASH_PAGE_SIZE - addr % MOD_FLASH_PAGE_SIZE; // addr 所在页的剩余页大小
    if (num_remain < num_write)
    {
        return ERROR; // 写入数据大于剩余页大小
    }

    Mod_Flash_Write_Enable();
    Mod_Flash_COM_Start();
    Mod_Flash_Send_Byte(MOD_FLASH_W25Q64_PAGE_PROGRAM);
    Mod_Flash_Send_Addr(addr);
    for (uint16_t i = 0; i < num_write; ++i)
    {
        Mod_Flash_Send_Byte(pbuffer[i]);
    }
    Mod_Flash_COM_Stop();
    Mod_Flash_Wait_Busy();
    return SUCCESS;
}

// 不定量写入
void Mod_Flash_Write(const uint8_t *const pbuffer, const uint32_t addr, const uint32_t num_write)
{
    uint32_t num_pages = 0, num_front = 0, num_tail = 0;
    uint32_t pb = (uint32_t)pbuffer, pa = addr; // pb 指向缓冲区, pa 指向 Flash 地址

    num_front = MOD_FLASH_PAGE_SIZE - addr % MOD_FLASH_PAGE_SIZE; // 头部部分页大小
    if (num_front >= num_write)                                   // 不需要跨页
    {
        (void)num_pages;
        (void)num_tail;
        (void)pb;
        (void)pa;
        Mod_Flash_Write_Page(pbuffer, addr, num_write);
    }
    else // 需要跨页
    {
        num_pages = (num_write - num_front) / MOD_FLASH_PAGE_SIZE; // 按完整页写入的页数
        num_tail = (num_write - num_front) % MOD_FLASH_PAGE_SIZE;  // 尾部剩余的部分页
        // 写入头部部分页
        Mod_Flash_Write_Page((uint8_t *)pb, pa, num_front);
        pb += num_front;
        pa += num_front;
        // 写入完整页
        for (uint32_t i = 0; i < num_pages; ++i)
        {
            Mod_Flash_Write_Page((uint8_t *)pb, pa, MOD_FLASH_PAGE_SIZE);
            pb += MOD_FLASH_PAGE_SIZE;
            pa += MOD_FLASH_PAGE_SIZE;
        }
        // 写入尾部剩余剩余页
        if (num_tail > 0)
        {
            Mod_Flash_Write_Page((uint8_t *)pb, pa, num_tail);
        }
    }
}

// 读取 Flash 没有地址对齐的要求
void Mod_Flash_Read(uint8_t *const pbuffer, const uint32_t addr, const uint32_t num_read)
{
    Mod_Flash_COM_Start();
    Mod_Flash_Send_Byte(MOD_FLASH_W25Q64_READ_DATA); // flash 开始读取, 就会一直发送数据, 直到通信结束
    Mod_Flash_Send_Addr(addr);
    for (uint32_t i = 0; i < num_read; ++i)
    {
        pbuffer[i] = Mod_Flash_Receive_Byte();
    }
    Mod_Flash_COM_Stop();
}
