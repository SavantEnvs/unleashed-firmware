// Functional oracle for FatFs: format a RAM disk (FAT16 and exFAT), write a small tree, read it back,
// and print what the library reports. test.sh diffs this output against golden values.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ff.h"
#include "fatfs_ramdisk.h"

static FATFS fs;
static uint8_t work[4096 * 4];

static int format(BYTE opt, size_t disk_size) {
    g_ramdisk = calloc(1, disk_size);
    g_ramdisk_size = disk_size;
    memset(&fs, 0, sizeof(fs));
    if(f_mkfs("", opt, 0, work, sizeof(work)) != FR_OK) return 1;
    if(f_mount(&fs, "", 1) != FR_OK) return 2;
    return 0;
}

static int report(void) {
    FILINFO fno;
    DIR dir;
    FIL f;
    char buf[64];
    UINT br;
    if(f_opendir(&dir, "/") != FR_OK) return 1;
    while(f_readdir(&dir, &fno) == FR_OK && fno.fname[0]) {
        printf("%s %s %lu\n", (fno.fattrib & AM_DIR) ? "D" : "F", fno.fname, (unsigned long)fno.fsize);
    }
    f_closedir(&dir);
    if(f_open(&f, "/dir_one/hello.txt", FA_READ) != FR_OK) return 2;
    memset(buf, 0, sizeof(buf));
    f_read(&f, buf, sizeof(buf) - 1, &br);
    f_close(&f);
    printf("hello.txt: %u bytes: %s", br, buf);
    if(f_open(&f, "/A_Rather_Long_File_Name_For_LFN.sub", FA_READ) != FR_OK) return 3;
    f_lseek(&f, 8 * 17);
    memset(buf, 0, sizeof(buf));
    f_read(&f, buf, 8, &br);
    printf("long.sub@136: %.8s size=%lu\n", buf, (unsigned long)f_size(&f));
    f_close(&f);
    DWORD nclst;
    FATFS* fsp;
    if(f_getfree("", &nclst, &fsp) != FR_OK) return 4;
    printf("fs_type=%d csize=%u\n", fs.fs_type, (unsigned)fs.csize);
    if(f_unlink("/dir_one/hello.txt") != FR_OK) return 5;
    printf("after unlink: stat=%d\n", (int)f_stat("/dir_one/hello.txt", &fno));
    return 0;
}

int main(void) {
    int rc;
    printf("== FAT ==\n");
    if((rc = format(FM_FAT, 4u * 1024 * 1024)) || (rc = fatfs_populate()) || (rc = report())) {
        printf("FAIL rc=%d\n", rc);
        return 1;
    }
    f_mount(NULL, "", 0);
    free(g_ramdisk);
    printf("== exFAT ==\n");
    if((rc = format(FM_EXFAT, 8u * 1024 * 1024)) || (rc = fatfs_populate()) || (rc = report())) {
        printf("FAIL rc=%d\n", rc);
        return 1;
    }
    f_mount(NULL, "", 0);
    free(g_ramdisk);
    return 0;
}
