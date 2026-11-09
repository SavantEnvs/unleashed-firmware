// Known-answer test for the TextBox module: with HOST_TRACE the stubbed element helpers print what the real
// ones would draw (first visible line, scrollbar position), so each scenario's output is a behavior to compare.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <furi.h>
#include <gui/view_i.h>
#include <gui/modules/text_box.h>
#include "shim_gui/host_stubs.h"

static const char* twelve =
    "line01\nline02\nline03\nline04\nline05\nline06\nline07\nline08\nline09\nline10\nline11\nline12";

static TextBox* make(const char* text, TextBoxFocus focus) {
    TextBox* tb = text_box_alloc();
    text_box_set_focus(tb, focus);
    text_box_set_text(tb, text);
    return tb;
}

static void press(TextBox* tb, InputKey key, InputType type) {
    InputEvent e = {.sequence = 0, .key = key, .type = type};
    view_input(text_box_get_view(tb), &e);
    view_draw(text_box_get_view(tb), host_canvas());
}

int main(void) {
    setvbuf(stdout, NULL, _IOLBF, 0);
    TextBox* tb;

    puts("case short-start");
    tb = make("alpha\nbeta\ngamma", TextBoxFocusStart);
    view_draw(text_box_get_view(tb), host_canvas());
    text_box_free(tb);

    puts("case twelve-start");
    tb = make(twelve, TextBoxFocusStart);
    view_draw(text_box_get_view(tb), host_canvas());
    text_box_free(tb);

    puts("case twelve-end");
    tb = make(twelve, TextBoxFocusEnd);
    view_draw(text_box_get_view(tb), host_canvas());
    text_box_free(tb);

    puts("case wrapped");
    char* a = malloc(151);
    memset(a, 'a', 150);
    a[150] = '\0';
    tb = make(a, TextBoxFocusStart);
    view_draw(text_box_get_view(tb), host_canvas());
    text_box_free(tb);
    free(a);

    puts("case scroll");
    tb = make(twelve, TextBoxFocusStart);
    view_draw(text_box_get_view(tb), host_canvas());
    press(tb, InputKeyDown, InputTypeShort);
    press(tb, InputKeyDown, InputTypeShort);
    press(tb, InputKeyDown, InputTypeShort);
    press(tb, InputKeyUp, InputTypeShort);
    text_box_free(tb);
    return 0;
}
