#pragma once
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef UNUSED
#define UNUSED(X) (void)(X)
#endif
#define furi_assert(expr) assert(expr)
#define furi_check(expr) assert(expr)
