#pragma once
// Host stand-in for the firmware's <furi.h>: just what gui/modules/text_box.c, gui/view.c and furi/core/string.c use.
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "host_defs.h"
#include <furi/core/string.h>

#ifndef UNUSED
#define UNUSED(X) (void)(X)
#endif
#ifndef MAX
#define MAX(a, b)               \
    ({                          \
        __typeof__(a) _a = (a); \
        __typeof__(b) _b = (b); \
        _a > _b ? _a : _b;      \
    })
#endif
#ifndef MIN
#define MIN(a, b)               \
    ({                          \
        __typeof__(a) _a = (a); \
        __typeof__(b) _b = (b); \
        _a < _b ? _a : _b;      \
    })
#endif

#define furi_assert(expr) \
    do {                  \
        if(!(expr)) abort(); \
    } while(0)
#define furi_check(expr) \
    do {                 \
        if(!(expr)) abort(); \
    } while(0)
#define furi_crash() abort()

typedef struct FuriMutex FuriMutex;
typedef enum { FuriMutexTypeNormal, FuriMutexTypeRecursive } FuriMutexType;
typedef enum { FuriStatusOk = 0, FuriStatusError = -1 } FuriStatus;
#define FuriWaitForever 0xFFFFFFFFU

FuriMutex* furi_mutex_alloc(FuriMutexType type);
void furi_mutex_free(FuriMutex* mutex);
FuriStatus furi_mutex_acquire(FuriMutex* mutex, uint32_t timeout);
FuriStatus furi_mutex_release(FuriMutex* mutex);
