// Functional oracle for the mJS interpreter: run known scripts and print the value each evaluates to.
// test.sh diffs this output against golden values.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mjs_core_public.h"
#include "mjs_exec_public.h"
#include "mjs_util_public.h"
#include "mjs_primitive_public.h"

static const char* const scripts[] = {
    "1 + 2 * 3",
    "(7 - 2) * (3 + 4) % 5",
    "'foo' + 'bar'",
    "'flipper zero'.slice(0, 7)",
    "'flipper zero'.indexOf('zero')",
    "let s = 0; for (let i = 1; i <= 100; i++) { s += i; } s",
    "function fib(n) { return n < 2 ? n : fib(n - 1) + fib(n - 2); } fib(15)",
    "let a = [10, 20, 30]; a.push(40); a.length * 1000 + a[3]",
    "let o = {x: 4, y: 5}; o.x * o.y",
    "(255).toString(16)",
    "(5).toString(2)",
    "'abc'.charCodeAt(1)",
    "'abc'.at(2)",
    "let n = 0; let k = 10; while (k > 0) { if (k % 2 === 0) { n += k; } k--; } n",
    "(100).toString(2)",
    "typeof 'x' + typeof 3 + typeof {}",
    "let t = [3, 1, 2]; t[1] = 9; t[0] + t[1] + t[2]",
    "isNaN(NaN)",
    "0.5 + 0.25",
    "chr(65) + chr(66)",
    "'abc'.length",
    "undefined",
    "1 +",
};

int main(void) {
    for(size_t i = 0; i < sizeof(scripts) / sizeof(scripts[0]); i++) {
        struct mjs* mjs = mjs_create(NULL);
        mjs_val_t res = mjs_mk_undefined();
        mjs_err_t err = mjs_exec(mjs, scripts[i], &res);
        if(err != MJS_OK) {
            printf("[%zu] error %d\n", i, (int)err);
        } else {
            char* p = NULL;
            size_t len = 0;
            int need_free = 0;
            if(mjs_to_string(mjs, &res, &p, &len, &need_free) == MJS_OK) {
                printf("[%zu] %.*s\n", i, (int)len, p);
                if(need_free) free(p);
            } else {
                printf("[%zu] <non-string>\n", i);
            }
        }
        mjs_destroy(mjs);
    }
    return 0;
}
