// libFuzzer harness for heatshrink (lib/heatshrink, used by lib/toolbox/compress.c): byte 0/1 pick the
// window/lookahead bits, byte 2 the decoder input buffer; the rest is both decoded as a compressed
// stream and compressed + decompressed again, which must reproduce the input.
#include <stdlib.h>
#include "hs_codec.h"

int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    if(size < 4 || size > 4096) return -1;
    uint8_t w = HEATSHRINK_MIN_WINDOW_BITS + data[0] % (HEATSHRINK_MAX_WINDOW_BITS - HEATSHRINK_MIN_WINDOW_BITS + 1);
    uint8_t l = HEATSHRINK_MIN_LOOKAHEAD_BITS + data[1] % (w - HEATSHRINK_MIN_LOOKAHEAD_BITS);
    uint16_t ibs = 1 + data[2];
    const uint8_t* body = data + 3;
    size_t n = size - 3;

    HsBuf dec = {0, 0};
    hs_decode(body, n, w, l, ibs, &dec);
    free(dec.data);

    HsBuf enc = {0, 0};
    if(hs_encode(body, n, w, l, &enc) == 0) {
        HsBuf back = {0, 0};
        if(hs_decode(enc.data, enc.len, w, l, ibs, &back) != 0 || back.len < n ||
           memcmp(back.data, body, n) != 0)
            abort();
        free(back.data);
    }
    free(enc.data);
    return 0;
}
