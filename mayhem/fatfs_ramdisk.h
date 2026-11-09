#pragma once
#include <stddef.h>
#include <stdint.h>

extern uint8_t* g_ramdisk;
extern size_t g_ramdisk_size;

// Writes a small directory tree (dir_one/hello.txt and a long-named file) to the mounted volume; 0 on success.
int fatfs_populate(void);
