// In-memory stream for microtar (lib/microtar): read-only over a caller buffer, or a growable write buffer.
#pragma once
#include <stdlib.h>
#include <string.h>
#include "microtar.h"

typedef struct {
    unsigned char* buf;
    size_t len, cap, pos;
    int writable;
} MemStream;

static int ms_read(void* s, void* data, unsigned size) {
    MemStream* m = s;
    if(m->pos + size > m->len) return MTAR_EREADFAIL;
    memcpy(data, m->buf + m->pos, size);
    m->pos += size;
    return (int)size;
}
static int ms_write(void* s, const void* data, unsigned size) {
    MemStream* m = s;
    if(!m->writable) return MTAR_EWRITEFAIL;
    if(m->pos + size > m->cap) {
        while(m->pos + size > m->cap) m->cap = m->cap ? m->cap * 2 : 4096;
        m->buf = realloc(m->buf, m->cap);
    }
    if(m->pos > m->len) memset(m->buf + m->len, 0, m->pos - m->len);
    memcpy(m->buf + m->pos, data, size);
    m->pos += size;
    if(m->pos > m->len) m->len = m->pos;
    return (int)size;
}
static int ms_seek(void* s, unsigned pos) {
    MemStream* m = s;
    if(pos > m->len) return MTAR_ESEEKFAIL;
    m->pos = pos;
    return MTAR_ESUCCESS;
}
static int ms_close(void* s) {
    (void)s;
    return MTAR_ESUCCESS;
}
static const mtar_ops_t ms_ops = {ms_read, ms_write, ms_seek, ms_close};
