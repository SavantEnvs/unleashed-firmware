// RAM-backed FatFs disk I/O (replaces lib/fatfs/diskio.c + ff_gen_drv.c, which bind to the SD card driver).
#include "diskio.h"
#include "ff.h"
#include <stdio.h>
#include <string.h>
#include "fatfs_ramdisk.h"

uint8_t* g_ramdisk = NULL;
size_t g_ramdisk_size = 0;

DWORD get_fattime(void) {
    // 2024-01-02 03:04:06
    return ((DWORD)(2024 - 1980) << 25) | ((DWORD)1 << 21) | ((DWORD)2 << 16) | ((DWORD)3 << 11) |
           ((DWORD)4 << 5) | (6 >> 1);
}

DSTATUS disk_status(BYTE pdrv) {
    return (pdrv == 0 && g_ramdisk) ? 0 : STA_NOINIT;
}

DSTATUS disk_initialize(BYTE pdrv) {
    return disk_status(pdrv);
}

DRESULT disk_read(BYTE pdrv, BYTE* buff, DWORD sector, UINT count) {
    if(pdrv != 0 || !g_ramdisk) return RES_NOTRDY;
    if(((uint64_t)sector + count) * 512u > g_ramdisk_size) return RES_PARERR;
    memcpy(buff, g_ramdisk + (size_t)sector * 512u, (size_t)count * 512u);
    return RES_OK;
}

DRESULT disk_write(BYTE pdrv, const BYTE* buff, DWORD sector, UINT count) {
    if(pdrv != 0 || !g_ramdisk) return RES_NOTRDY;
    if(((uint64_t)sector + count) * 512u > g_ramdisk_size) return RES_PARERR;
    memcpy(g_ramdisk + (size_t)sector * 512u, buff, (size_t)count * 512u);
    return RES_OK;
}

DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void* buff) {
    if(pdrv != 0 || !g_ramdisk) return RES_NOTRDY;
    switch(cmd) {
    case CTRL_SYNC:
        return RES_OK;
    case GET_SECTOR_COUNT:
        *(DWORD*)buff = (DWORD)(g_ramdisk_size / 512u);
        return RES_OK;
    case GET_SECTOR_SIZE:
        *(WORD*)buff = 512;
        return RES_OK;
    case GET_BLOCK_SIZE:
        *(DWORD*)buff = 1;
        return RES_OK;
    default:
        return RES_PARERR;
    }
}

int fatfs_populate(void) {
    FIL f;
    UINT bw;
    static const char hello[] = "hello flipper\n";
    if(f_mkdir("/dir_one") != FR_OK) return 1;
    if(f_open(&f, "/dir_one/hello.txt", FA_CREATE_NEW | FA_WRITE) != FR_OK) return 2;
    if(f_write(&f, hello, sizeof(hello) - 1, &bw) != FR_OK || bw != sizeof(hello) - 1) return 3;
    f_close(&f);
    if(f_open(&f, "/A_Rather_Long_File_Name_For_LFN.sub", FA_CREATE_NEW | FA_WRITE) != FR_OK) return 4;
    for(int i = 0; i < 40; i++) {
        char line[32];
        int n = snprintf(line, sizeof(line), "line %03d\n", i);
        if(f_write(&f, line, n, &bw) != FR_OK || bw != (UINT)n) return 5;
    }
    f_close(&f);
    return 0;
}
