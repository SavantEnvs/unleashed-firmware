// libFuzzer harness for the mJS interpreter (lib/mjs): the fuzz input is a script, run as-is.
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "mjs_core_public.h"
#include "mjs_exec_public.h"
#include "mjs_util_public.h"
#include "mjs_primitive_public.h"

int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    if(size == 0 || size > 4096) return -1;
    char* src = malloc(size + 1);
    if(!src) return -1;
    memcpy(src, data, size);
    src[size] = '\0';

    struct mjs* mjs = mjs_create(NULL);
    mjs_val_t res = mjs_mk_undefined();
    mjs_exec(mjs, src, &res);
    mjs_destroy(mjs);
    free(src);
    return 0;
}
