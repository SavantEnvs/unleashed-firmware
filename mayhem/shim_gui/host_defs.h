#pragma once
// Force-included (-include) in every host build: newlib's _ATTRIBUTE, which furi/core/string.h uses before it
// includes anything.
#ifndef _ATTRIBUTE
#define _ATTRIBUTE(a) __attribute__(a)
#endif

// The firmware's malloc() returns zeroed memory (furi/core/memmgr.c); the code relies on it
// (view_alloc, text_box_alloc), so the host build matches.
#include <stdlib.h>
#define malloc(n) calloc(1, (n))
