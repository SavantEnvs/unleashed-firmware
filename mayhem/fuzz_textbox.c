// libFuzzer harness for the GUI TextBox module (applications/services/gui/modules/text_box.c): an app hands
// the module a NUL-terminated string with text_box_set_text(), the GUI thread then draws the view (which
// formats the text into lines) and feeds it key presses.
//
// Input: byte 0 = font (bit 0) and focus (bit 1); byte 1 = number of key events (low 4 bits); then one byte per
// event (bit 0 Up/Down, bits 1-2 Short/Repeat/Release/Long); the rest is the text.
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <furi.h>
#include <gui/view_i.h>
#include <gui/modules/text_box.h>
#include "shim_gui/host_stubs.h"

int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    if(size < 2 || size > 16384) return -1;
    const size_t events = data[1] & 0x0F;
    if(size < 2 + events) return -1;
    const uint8_t* text_in = data + 2 + events;
    const size_t text_len = size - 2 - events;

    char* text = malloc(text_len + 1);
    if(!text) return -1;
    memcpy(text, text_in, text_len);
    text[text_len] = '\0';

    TextBox* text_box = text_box_alloc();
    text_box_set_font(text_box, (data[0] & 1) ? TextBoxFontHex : TextBoxFontText);
    text_box_set_focus(text_box, (data[0] & 2) ? TextBoxFocusEnd : TextBoxFocusStart);
    text_box_set_text(text_box, text);

    View* view = text_box_get_view(text_box);
    Canvas* canvas = host_canvas();
    view_draw(view, canvas);
    for(size_t i = 0; i < events; i++) {
        const uint8_t e = data[2 + i];
        InputEvent event = {
            .sequence = i,
            .key = (e & 1) ? InputKeyDown : InputKeyUp,
            .type = (InputType[]){InputTypeShort, InputTypeRepeat, InputTypeRelease, InputTypeLong}[(e >> 1) & 3],
        };
        view_input(view, &event);
        view_draw(view, canvas);
    }

    text_box_free(text_box);
    free(text);
    return 0;
}
