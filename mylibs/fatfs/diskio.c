/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2025        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"			/* Basic definitions of FatFs */
#include "diskio.h"		/* Declarations FatFs MAI */

/* Example: Declarations of the platform and disk functions in the project */
#include "lib_spi.h"
#include "mod_flash.h"

/* Example: Mapping of physical drive number for each drive */
#define DEV_FLASH	0	/* Map FTL to physical drive 0 */
// #define DEV_MMC		1	/* Map MMC/SD card to physical drive 1 */
// #define DEV_USB		2	/* Map USB MSD to physical drive 2 */


/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat = STA_NOINIT | STA_PROTECT; // FatFs 没有 STA_NODISK
	int result;

	switch (pdrv) {
	case DEV_FLASH:
		result = Mod_Flash_Read_JEDCE_ID();
		if (result == MOD_FLASH_JEDEC_ID)
		{
			(void)stat;
			return ~stat; // 必须清除两个位, 因为 f_mkfs() 会检测这两个位
		}
		break;
	}
	return STA_NOINIT;
}

/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat;
	int result;

	switch (pdrv) {
	case DEV_FLASH:
		Lib_SPI_Init();
		(void)stat;
		(void)result;
		return disk_status(pdrv);
	}
	return STA_NOINIT;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	DRESULT res;
	int result;

	switch (pdrv) {
	case DEV_FLASH:
		// sector 是扇区的标号, 不是实际地址; 因此需要乘以 4096, 即左移 12 位
		// 同理, count 是读取的扇区数, 不是字节数, 也要左移 12 位
		Mod_Flash_Read((uint8_t*)buff, sector << 12, count << 12);
		(void)res;
		(void)result;
		return RES_OK;
	}

	return RES_PARERR;
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	LBA_t sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	DRESULT res;
	int result;

	switch (pdrv) {
	case DEV_FLASH:
		// sector 为扇区的编号, 不是真实地址, 需要乘以 4096, 即左移 12 位
		// count 为写入扇区的数量, 也要左移 12 位表示字节数
		for (uint8_t i = 0; i < count; ++i)
			Mod_Flash_Erase_Sector((sector + i) << 12);
		Mod_Flash_Write((uint8_t*)buff, sector << 12, count << 12);
		(void)res;
		(void)result;
		return RES_OK;
	}

	return RES_PARERR;
}

#endif

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res;
	int result;

	switch (pdrv) {
	case DEV_FLASH:
		switch (cmd)
		{	
			(void)res;
			(void)result;
			// 将存储器缓存的数据立刻写入物理介质
			case CTRL_SYNC:
				// Mod_Flash_Write() 保证数据一定写入物理介质
				break;
			// 获取扇区数量, 该指令需要 UINT 参数
			case GET_SECTOR_COUNT:
				// 使用的 Flash 一共有 2048 个扇区
				*(UINT*)buff = 2048;
				break;
			// 获取扇区大小, 该指令需要 UINT 参数
			case GET_SECTOR_SIZE:
				// 使用的 Flash 的大小为 4096 B
				*(UINT*)buff = 4096;
				break;
			// 获取擦除块大小, 该指令需要 UINT 参数
			case GET_BLOCK_SIZE:
				// 擦除块大小的意思是存储设备最小的擦除单位是几个扇区
				// 使用的 Flash 能够逐扇区擦除, 即 1
				*(UINT*)buff = 1;
				break;
			default:
				return RES_PARERR;
		}
		return RES_OK;
	}

	return RES_PARERR;
}
