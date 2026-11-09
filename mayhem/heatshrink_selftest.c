// Oracle: known-answer compress/decompress round trips through lib/heatshrink.
#include <stdio.h>
#include "hs_codec.h"

static void hex(const uint8_t* p, size_t n) {
    for(size_t i = 0; i < n; i++) printf("%02x", p[i]);
}

static int run(const char* name, const char* text, uint8_t w, uint8_t l) {
    HsBuf enc = {0, 0}, back = {0, 0};
    size_t n = strlen(text);
    if(hs_encode((const uint8_t*)text, n, w, l, &enc) != 0) { printf("%s: encode error\n", name); return 1; }
    printf("%s: %zu -> %zu ", name, n, enc.len);
    hex(enc.data, enc.len);
    int rc = hs_decode(enc.data, enc.len, w, l, 64, &back);
    int ok = rc == 0 && back.len >= n && memcmp(back.data, text, n) == 0;
    printf(" roundtrip=%s\n", ok ? "ok" : "BAD");
    free(enc.data);
    free(back.data);
    return !ok;
}

int main(void) {
    int bad = 0;
    bad += run("short", "hello", 8, 4);
    bad += run("repeat", "abcabcabcabcabcabcabcabcabcabc", 8, 4);
    bad += run("flipper", "flipper flipper flipper zero zero zero flipper", 10, 5);
    bad += run("zeros", "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", 4, 3);
    printf("bad=%d\n", bad);
    return bad;
}
