// Host replacements for lib/mjs/common/platforms/platform_flipper.c (which needs the Flipper storage service).
#include <stdarg.h>
#include <stddef.h>
#include "common/cs_dbg.h"
#include "common/frozen/frozen.h"

char* cs_read_file(const char* path, size_t* size) {
    (void)path;
    (void)size;
    return NULL;
}

char* json_fread(const char* path) {
    (void)path;
    return NULL;
}

int json_vfprintf(const char* file_name, const char* fmt, va_list ap) {
    (void)file_name;
    (void)fmt;
    (void)ap;
    return 0;
}

int json_prettify_file(const char* file_name) {
    (void)file_name;
    return 0;
}

int json_printer_file(struct json_out* out, const char* buf, size_t len) {
    (void)out;
    (void)buf;
    (void)len;
    return 0;
}

int cs_log_print_prefix(enum cs_log_level level, const char* file, int ln) {
    (void)level;
    (void)file;
    (void)ln;
    return 0;
}

void cs_log_printf(const char* fmt, ...) {
    (void)fmt;
}
