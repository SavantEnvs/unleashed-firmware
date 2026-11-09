// libFuzzer harness for microtar (lib/microtar, used for .tar extraction): the input is a tar archive,
// walked entry by entry with every entry's data read, then searched by name.
#include <stdint.h>
#include <stdio.h>
#include "mtar_mem.h"

static int visit(mtar_t* tar, const mtar_header_t* h, void* arg) {
    (void)arg;
    unsigned remaining = h->size;
    char buf[300];
    while(remaining > 0) {
        unsigned chunk = remaining < sizeof buf ? remaining : sizeof buf;
        if(mtar_read_data(tar, buf, chunk) < 0) break;
        remaining -= chunk;
    }
    mtar_seek_data(tar, 0, SEEK_SET);
    return 0;
}

int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    if(size == 0 || size > 8192) return -1;
    MemStream ms = {(unsigned char*)data, size, size, 0, 0};
    mtar_t tar;
    mtar_init(&tar, MTAR_READ, &ms_ops, &ms);
    mtar_foreach(&tar, visit, NULL);
    mtar_rewind(&tar);
    mtar_find(&tar, "flipper.txt");
    mtar_close(&tar);
    return 0;
}
