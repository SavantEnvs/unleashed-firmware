// Host stubs for the firmware pieces text_box.c leans on: the canvas (no display; glyph widths are a fixed
// function of the symbol, 3..10 px like the real fonts), the element drawing helpers (they only read the text
// they are given, as the real ones do), and a recursive mutex. gui/view.c, gui/modules/text_box.c and
// furi/core/string.c (with the pre-fetched mlib) are the firmware's own sources.
#include <furi.h>
#include <gui/canvas.h>
#include <gui/elements.h>
#include <gui/icon_animation.h>
#include "host_stubs.h"

struct Canvas {
    Font font;
};

static struct Canvas g_canvas;

Canvas* host_canvas(void) {
    g_canvas.font = FontPrimary;
    return &g_canvas;
}

void canvas_clear(Canvas* canvas) {
    UNUSED(canvas);
}

void canvas_set_font(Canvas* canvas, Font font) {
    canvas->font = font;
}

size_t canvas_glyph_width(Canvas* canvas, uint16_t symbol) {
    UNUSED(canvas);
    return 3 + (symbol & 7);
}

// 11 px lines give the 5 lines on screen that the code before the fix hard-codes.
size_t canvas_current_font_height(const Canvas* canvas) {
    UNUSED(canvas);
    return 11;
}

void elements_slightly_rounded_frame(
    Canvas* canvas,
    int32_t x,
    int32_t y,
    size_t width,
    size_t height) {
    UNUSED(canvas);
    UNUSED(x);
    UNUSED(y);
    UNUSED(width);
    UNUSED(height);
}

void elements_multiline_text(Canvas* canvas, int32_t x, int32_t y, const char* text) {
    UNUSED(canvas);
    UNUSED(x);
    UNUSED(y);
    // The screen shows six lines at most.
    size_t n = 0;
    int lines = 0;
    while(text[n] != '\0' && lines < 6) {
        if(text[n++] == '\n') lines++;
    }
#ifdef HOST_TRACE
    size_t first = 0;
    while(first < n && text[first] != '\n') first++;
    printf("text: %.*s\n", (int)first, text);
#endif
}

void elements_scrollbar(Canvas* canvas, size_t pos, size_t total) {
    UNUSED(canvas);
    UNUSED(pos);
    UNUSED(total);
#ifdef HOST_TRACE
    printf("scrollbar: %ld/%ld\n", (long)(int32_t)pos, (long)(int32_t)total);
#endif
}

struct FuriMutex {
    int depth;
};

FuriMutex* furi_mutex_alloc(FuriMutexType type) {
    UNUSED(type);
    return calloc(1, sizeof(FuriMutex));
}

void furi_mutex_free(FuriMutex* mutex) {
    free(mutex);
}

FuriStatus furi_mutex_acquire(FuriMutex* mutex, uint32_t timeout) {
    UNUSED(timeout);
    mutex->depth++;
    return FuriStatusOk;
}

FuriStatus furi_mutex_release(FuriMutex* mutex) {
    if(mutex->depth <= 0) return FuriStatusError;
    mutex->depth--;
    return FuriStatusOk;
}

// Linked in by gui/view.c's view_tie_icon_animation; the TextBox has no icons.
void icon_animation_set_update_callback(
    IconAnimation* instance,
    IconAnimationCallback callback,
    void* context) {
    UNUSED(instance);
    UNUSED(callback);
    UNUSED(context);
}
