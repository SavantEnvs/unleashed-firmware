// A scripted EMV card for lib/nfc/protocols/emv/emv_poller_i.c: the poller's own entry points
// (emv_poller.h) run in the order emv_poller.c's handlers call them, and the ISO14443-4A transport
// beneath them hands back the card's replies from a byte script: [length][reply bytes] ... Once the
// script is used up the card is gone (a timeout), as when a real card leaves the field.
#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <lib/nfc/protocols/emv/emv_poller_i.h>

#define EMV_BUF_SIZE 512U // emv_poller.c

static const uint8_t* script_ptr;
static size_t script_left;

Iso14443_4aError iso14443_4a_poller_send_block_pwt_ext(
    Iso14443_4aPoller* instance,
    const BitBuffer* tx_buffer,
    BitBuffer* rx_buffer) {
    (void)instance;
    (void)tx_buffer;
    if(script_left == 0) return Iso14443_4aErrorTimeout;
    size_t n = script_ptr[0];
    script_ptr++;
    script_left--;
    if(n > script_left) n = script_left;
    bit_buffer_copy_bytes(rx_buffer, script_ptr, n);
    script_ptr += n;
    script_left -= n;
    return Iso14443_4aErrorNone;
}

static EmvPoller* emv_session_alloc(void) {
    EmvPoller* p = calloc(1, sizeof(EmvPoller));
    p->iso14443_4a_poller = (Iso14443_4aPoller*)p;
    p->data = calloc(1, sizeof(EmvData));
    p->data->emv_application.pin_try_counter = 0xff; // emv_alloc()
    p->tx_buffer = bit_buffer_alloc(EMV_BUF_SIZE);
    p->rx_buffer = bit_buffer_alloc(EMV_BUF_SIZE);
    return p;
}

static void emv_session_free(EmvPoller* p) {
    bit_buffer_free(p->tx_buffer);
    bit_buffer_free(p->rx_buffer);
    free(p->data);
    free(p);
}

// emv_poller_handler_select_ppse .. emv_poller_handler_read_extra_data
static void emv_session_run(EmvPoller* p, const uint8_t* script, size_t size) {
    script_ptr = script;
    script_left = size;
    if(emv_poller_select_ppse(p) != EmvErrorNone) return;
    emv_poller_select_application(p);
    emv_poller_get_processing_options(p);
    emv_poller_read_afl(p, false, &p->records_mask);
    emv_poller_read_log_entry(p);
    emv_poller_get_last_online_atc(p);
    emv_poller_get_pin_try_counter(p);
    emv_poller_read_afl(p, true, &p->records_mask);
}
