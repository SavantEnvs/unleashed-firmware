// libFuzzer harness for FatFs (lib/fatfs, R0.12c with exFAT).
//
// A raw disk image is too large for libFuzzer's input size (a formatted exFAT volume is ~50 KiB), so the input
// is a compact edit list applied to a valid, populated volume that is formatted once at startup:
//   byte 0      : bit 0 selects the base volume (0 = FAT, 1 = exFAT)
//   bytes 1..   : 3-byte records {offset_lo, offset_hi, value}, each setting disk[offset] = value
// The edited image is then mounted, walked, read, and written to through the real f_* API.
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ff.h"
#include "fatfs_ramdisk.h"

#define DISK_SIZE (3072u * 1024u)
#define MAX_ENTRIES 128
#define MAX_DEPTH 4

static uint8_t disk[DISK_SIZE];
static uint8_t base_image[2][DISK_SIZE];
static FATFS fs;
static int entries_seen;

int LLVMFuzzerInitialize(int* argc, char*** argv) {
    (void)argc;
    (void)argv;
    static const BYTE opts[2] = {FM_FAT, FM_EXFAT};
    static uint8_t work[4096 * 4];
    for(int i = 0; i < 2; i++) {
        g_ramdisk = base_image[i];
        g_ramdisk_size = DISK_SIZE;
        memset(&fs, 0, sizeof(fs));
        FRESULT r1 = f_mkfs("", opts[i], 0, work, sizeof(work));
        FRESULT r2 = r1 ? r1 : f_mount(&fs, "", 1);
        int r3 = r2 ? -1 : fatfs_populate();
        if(r1 || r2 || r3) {
            fprintf(stderr, "fuzz_fatfs: cannot build base volume %d (%d %d %d)\n", i, r1, r2, r3);
            abort();
        }
        f_mount(NULL, "", 0);
    }
    return 0;
}

static void read_file(const char* path) {
    FIL f;
    uint8_t buf[512];
    UINT br;
    if(f_open(&f, path, FA_READ) != FR_OK) return;
    if(f_read(&f, buf, sizeof(buf), &br) == FR_OK && br == sizeof(buf)) {
        FSIZE_t mid = f_size(&f) / 2;
        if(f_lseek(&f, mid) == FR_OK) f_read(&f, buf, sizeof(buf), &br);
    }
    f_close(&f);
}

static void walk(const char* path, int depth) {
    DIR dir;
    FILINFO fno;
    if(f_opendir(&dir, path) != FR_OK) return;
    while(entries_seen < MAX_ENTRIES) {
        if(f_readdir(&dir, &fno) != FR_OK || fno.fname[0] == 0) break;
        entries_seen++;
        char child[300];
        if(snprintf(child, sizeof(child), "%s/%s", path, fno.fname) >= (int)sizeof(child)) continue;
        if(fno.fattrib & AM_DIR) {
            if(depth < MAX_DEPTH) walk(child, depth + 1);
        } else {
            f_stat(child, &fno);
            read_file(child);
        }
    }
    f_closedir(&dir);
}

int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    if(size < 1) return -1;
    memcpy(disk, base_image[data[0] & 1], DISK_SIZE);
    for(size_t i = 1; i + 3 <= size; i += 3) {
        disk[(size_t)data[i] | ((size_t)data[i + 1] << 8)] = data[i + 2];
    }
    g_ramdisk = disk;
    g_ramdisk_size = DISK_SIZE;
    entries_seen = 0;

    memset(&fs, 0, sizeof(fs));
    if(f_mount(&fs, "", 1) != FR_OK) {
        f_mount(NULL, "", 0);
        return -1;
    }

    DWORD nclst;
    FATFS* fsp;
    f_getfree("", &nclst, &fsp);
    walk("", 0);

    FIL f;
    UINT bw;
    uint8_t payload[1500];
    for(size_t i = 0; i < sizeof(payload); i++) payload[i] = (uint8_t)i;
    if(f_open(&f, "/mayhem_fuzz.bin", FA_CREATE_ALWAYS | FA_WRITE | FA_READ) == FR_OK) {
        f_write(&f, payload, sizeof(payload), &bw);
        f_sync(&f);
        f_close(&f);
    }
    if(f_mkdir("/mayhem_dir") == FR_OK) f_rename("/mayhem_fuzz.bin", "/mayhem_dir/renamed.bin");
    f_unlink("/mayhem_dir/renamed.bin");
    f_mount(NULL, "", 0);
    return 0;
}
