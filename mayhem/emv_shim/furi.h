// Host stand-in for the firmware's <furi.h>: just what bit_buffer.c and emv_poller_i.c use.
#pragma once
#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef UNUSED
#define UNUSED(X) (void)(X)
#endif
#define furi_assert(expr) assert(expr)
#define furi_check(expr)  assert(expr)

#define COUNT_OF(x) (sizeof(x) / sizeof(x[0]))
#define MIN(a, b) \
    ({ \
        __typeof__(a) _a = (a); \
        __typeof__(b) _b = (b); \
        _a < _b ? _a : _b; \
    })
#define FURI_BIT(x, n)     (((x) >> (n)) & 1)
#define FURI_BIT_SET(x, n) ((x) |= (1UL << (n)))

#define FURI_LOG_E(tag, ...)    ((void)(tag))
#define FURI_LOG_W(tag, ...)    ((void)(tag))
#define FURI_LOG_I(tag, ...)    ((void)(tag))
#define FURI_LOG_D(tag, ...)    ((void)(tag))
#define FURI_LOG_T(tag, ...)    ((void)(tag))
#define FURI_LOG_RAW_T(...)     ((void)0)

typedef enum { FuriLogLevelDefault, FuriLogLevelNone, FuriLogLevelError, FuriLogLevelWarn, FuriLogLevelInfo, FuriLogLevelDebug, FuriLogLevelTrace } FuriLogLevel;
static inline FuriLogLevel furi_log_get_level(void) {
    return FuriLogLevelInfo;
}

typedef struct FuriString {
    char s[96];
} FuriString;
static inline FuriString* furi_string_alloc(void) {
    return calloc(1, sizeof(FuriString));
}
static inline void furi_string_free(FuriString* f) {
    free(f);
}
static inline const char* furi_string_get_cstr(const FuriString* f) {
    return f->s;
}
static inline void furi_string_printf(FuriString* f, const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(f->s, sizeof(f->s), fmt, ap);
    va_end(ap);
}
