// Whole-buffer wrappers over the heatshrink streaming encoder/decoder (lib/heatshrink).
#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "heatshrink_encoder.h"
#include "heatshrink_decoder.h"

#define HS_OUT_LIMIT (1u << 20)

typedef struct {
    uint8_t* data;
    size_t len;
} HsBuf;

static void hs_append(HsBuf* b, const uint8_t* p, size_t n, size_t* cap) {
    if(n == 0) return;
    if(b->len + n > *cap) {
        while(b->len + n > *cap) *cap = *cap ? *cap * 2 : 256;
        b->data = realloc(b->data, *cap);
    }
    memcpy(b->data + b->len, p, n);
    b->len += n;
}

// Returns 0 on success, -1 on a parameter or state error.
static int hs_encode(const uint8_t* in, size_t n, uint8_t w, uint8_t l, HsBuf* out) {
    heatshrink_encoder* e = heatshrink_encoder_alloc(w, l);
    if(!e) return -1;
    size_t cap = 0, off = 0;
    uint8_t tmp[64];
    int rc = 0;
    while(off < n) {
        size_t sunk = 0;
        if(heatshrink_encoder_sink(e, (uint8_t*)in + off, n - off, &sunk) < 0) { rc = -1; break; }
        off += sunk;
        HSE_poll_res pr;
        do {
            size_t got = 0;
            pr = heatshrink_encoder_poll(e, tmp, sizeof tmp, &got);
            if(pr < 0) { rc = -1; break; }
            hs_append(out, tmp, got, &cap);
        } while(pr == HSER_POLL_MORE);
        if(rc) break;
    }
    while(rc == 0 && heatshrink_encoder_finish(e) == HSER_FINISH_MORE) {
        size_t got = 0;
        if(heatshrink_encoder_poll(e, tmp, sizeof tmp, &got) < 0) { rc = -1; break; }
        hs_append(out, tmp, got, &cap);
    }
    heatshrink_encoder_free(e);
    return rc;
}

static int hs_decode(const uint8_t* in, size_t n, uint8_t w, uint8_t l, uint16_t ibs, HsBuf* out) {
    heatshrink_decoder* d = heatshrink_decoder_alloc(ibs, w, l);
    if(!d) return -1;
    size_t cap = 0, off = 0;
    uint8_t tmp[64];
    int rc = 0;
    while(off < n && rc == 0) {
        size_t sunk = 0;
        if(heatshrink_decoder_sink(d, (uint8_t*)in + off, n - off, &sunk) < 0) { rc = -1; break; }
        off += sunk;
        HSD_poll_res pr;
        do {
            size_t got = 0;
            pr = heatshrink_decoder_poll(d, tmp, sizeof tmp, &got);
            if(pr < 0) { rc = -1; break; }
            hs_append(out, tmp, got, &cap);
            if(out->len > HS_OUT_LIMIT) { rc = -1; break; }
        } while(pr == HSDR_POLL_MORE);
    }
    while(rc == 0 && heatshrink_decoder_finish(d) == HSDR_FINISH_MORE) {
        size_t got = 0;
        if(heatshrink_decoder_poll(d, tmp, sizeof tmp, &got) < 0) { rc = -1; break; }
        hs_append(out, tmp, got, &cap);
        if(out->len > HS_OUT_LIMIT) { rc = -1; break; }
    }
    heatshrink_decoder_free(d);
    return rc;
}
