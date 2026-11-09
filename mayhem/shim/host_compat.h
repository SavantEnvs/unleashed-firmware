#pragma once
// Force-included into every host-compiled firmware source: newlib's itoa() does not exist in glibc.
#include <stdint.h>

static inline char* itoa(int value, char* str, int base) {
    char tmp[34];
    if(base < 2 || base > 36) {
        str[0] = '\0';
        return str;
    }
    int n = 0;
    int neg = (value < 0 && base == 10);
    unsigned int u = neg ? 0u - (unsigned int)value : (unsigned int)value;
    do {
        unsigned int d = u % (unsigned int)base;
        tmp[n++] = (char)(d < 10 ? '0' + d : 'a' + d - 10);
        u /= (unsigned int)base;
    } while(u);
    int i = 0;
    if(neg) str[i++] = '-';
    while(n) str[i++] = tmp[--n];
    str[i] = '\0';
    return str;
}
