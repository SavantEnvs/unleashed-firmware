// Oracle: write a tar archive with microtar into memory, check its size/checksummed layout, read it back.
#include <stdio.h>
#include "mtar_mem.h"

static int list(mtar_t* t, const mtar_header_t* h, void* arg) {
    (void)arg;
    char buf[64] = {0};
    if(h->type == MTAR_TREG && h->size < sizeof buf) {
        mtar_read_data(t, buf, h->size);
    }
    printf("%c %s %u '%s'\n", (char)h->type, h->name, h->size, buf);
    return 0;
}

int main(void) {
    MemStream w = {0, 0, 0, 0, 1};
    mtar_t t;
    mtar_init(&t, MTAR_WRITE, &ms_ops, &w);
    printf("mkdir=%d\n", mtar_write_dir_header(&t, "dir"));
    printf("wh1=%d\n", mtar_write_file_header(&t, "dir/hello.txt", 13));
    printf("wd1=%d\n", mtar_write_data(&t, "hello flipper", 13));
    mtar_end_data(&t);
    printf("wh2=%d\n", mtar_write_file_header(&t, "flipper.txt", 4));
    printf("wd2=%d\n", mtar_write_data(&t, "zero", 4));
    mtar_end_data(&t);
    printf("fin=%d\n", mtar_finalize(&t));
    printf("size=%zu chksum=%.6s\n", w.len, (char*)w.buf + 148);

    MemStream r = {w.buf, w.len, w.len, 0, 0};
    mtar_init(&t, MTAR_READ, &ms_ops, &r);
    printf("foreach=%d\n", mtar_foreach(&t, list, NULL));
    mtar_rewind(&t);
    printf("find=%d\n", mtar_find(&t, "flipper.txt"));
    printf("find_missing=%d\n", mtar_find(&t, "nope"));
    free(w.buf);
    return 0;
}
