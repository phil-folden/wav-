#include "ff.h"
#include "diskio.h"
#include "HW226.h"
#include "Serial.h"

static volatile DSTATUS Stat = STA_NOINIT;

DSTATUS disk_initialize(BYTE pdrv)
{
    if (pdrv != 0)
    {
        return STA_NOINIT;
    }
    if (SD_Init() == 0)
    {
        Stat = 0;
    }
    else
    {
        Stat = STA_NOINIT;
    }

    return Stat;
}

DSTATUS disk_status(BYTE pdrv)
{
    if (pdrv != 0)
    {
        return STA_NOINIT;
    }

    return Stat;
}

DRESULT disk_read(BYTE pdrv, BYTE *buff, LBA_t sector, UINT count)
{
    UINT i;

    if (pdrv != 0)
    {
        return RES_PARERR;
    }

    if (buff == 0)
    {
        return RES_PARERR;
    }

    if (count == 0)
    {
        return RES_PARERR;
    }

    if (Stat & STA_NOINIT)
    {
        return RES_NOTRDY;
    }

    for (i = 0; i < count; i++)
    {
        if (SD_ReadSector((uint32_t)(sector + i), buff + i * 512) != 0)
        {
            return RES_ERROR;
        }
    }

    return RES_OK;
}

#if FF_FS_READONLY == 0

DRESULT disk_write(BYTE pdrv, const BYTE *buff, LBA_t sector, UINT count)
{
    return RES_ERROR;
}

#endif

DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void *buff)
{
    if (pdrv != 0)
    {
        return RES_PARERR;
    }

    if (Stat & STA_NOINIT)
    {
        return RES_NOTRDY;
    }

    switch (cmd)
    {
        case CTRL_SYNC:
            return RES_OK;

        case GET_SECTOR_SIZE:
            *(WORD *)buff = 512;
            return RES_OK;

        case GET_BLOCK_SIZE:
            *(DWORD *)buff = 1;
            return RES_OK;

        case GET_SECTOR_COUNT:
            /*
             * 临时写你这张卡第一个分区大小。
             * 后面更规范的做法是从 SD 卡 CSD 读容量。
             */
            *(DWORD *)buff = 0x001D73E0;
            return RES_OK;

        default:
            return RES_PARERR;
    }
}

DWORD get_fattime(void)
{
    /*
     * 返回一个固定时间：2026-05-03 12:00:00
     *
     * FatFs 时间格式：
     * bit31:25  年份，从 1980 开始
     * bit24:21  月份，1~12
     * bit20:16  日期，1~31
     * bit15:11  小时，0~23
     * bit10:5   分钟，0~59
     * bit4:0    秒 / 2，0~29
     */
    return ((DWORD)(2026 - 1980) << 25)
         | ((DWORD)5 << 21)
         | ((DWORD)3 << 16)
         | ((DWORD)12 << 11)
         | ((DWORD)0 << 5)
         | ((DWORD)0 >> 1);
}