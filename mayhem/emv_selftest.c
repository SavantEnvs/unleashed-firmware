// Known-answer test for the EMV poller: a scripted Mastercard-like card is read through the poller's
// entry points and what the poller decoded from its replies is printed, one fact per line.
#include <stdio.h>
#include "emv_session.h"

static void hex(const char* name, const uint8_t* d, size_t n) {
    printf("%s=", name);
    for(size_t i = 0; i < n; i++) printf("%02X", d[i]);
    printf("\n");
}

static const uint8_t card[] = {
    // select PPSE: FCI with a directory entry (AID, label, priority)
    33, 0x6F, 0x1F, 0xA5, 0x1D, 0xBF, 0x0C, 0x1A, 0x61, 0x18, 0x4F, 0x07, 0xA0, 0x00, 0x00, 0x00,
    0x04, 0x10, 0x10, 0x50, 0x0A, 'M', 'A', 'S', 'T', 'E', 'R', 'C', 'A', 'R', 'D', 0x87, 0x01, 0x01,
    // select application: label and a PDOL asking for amount and unpredictable number
    25, 0x6F, 0x17, 0xA5, 0x15, 0x50, 0x0A, 'M', 'A', 'S', 'T', 'E', 'R', 'C', 'A', 'R', 'D', 0x9F,
    0x38, 0x06, 0x9F, 0x02, 0x06, 0x9F, 0x37, 0x04,
    // GPO (format 2): AIP and the file locator
    12, 0x77, 0x0A, 0x82, 0x02, 0x19, 0x80, 0x94, 0x04, 0x08, 0x01, 0x01, 0x00,
    // READ RECORD SFI 1 record 1: PAN, expiry, cardholder name
    30, 0x70, 0x1C, 0x5A, 0x08, 0x41, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x5F, 0x24, 0x03,
    0x28, 0x12, 0x31, 0x5F, 0x20, 0x09, 'V', 'I', 'S', 'A', '/', 'T', 'E', 'S', 'T',
    // GET DATA last online ATC, PIN try counter
    5, 0x9F, 0x13, 0x02, 0x00, 0x2A,
    4, 0x9F, 0x17, 0x01, 0x03,
};

int main(void) {
    EmvPoller* p = emv_session_alloc();
    emv_session_run(p, card, sizeof(card));
    const EmvApplication* a = &p->data->emv_application;
    hex("aid", a->aid, a->aid_len);
    printf("label=%s\n", a->application_label);
    printf("priority=%u\n", a->priority);
    hex("pdol", a->pdol.data, a->pdol.size);
    hex("aip", a->application_interchange_profile, 2);
    hex("afl", a->afl.data, a->afl.size);
    hex("pan", a->pan, a->pan_len);
    printf("expiry=%02X%02X\n", a->exp_year, a->exp_month);
    printf("name=%s\n", a->cardholder_name);
    printf("last_online_atc=%u\n", a->last_online_atc);
    printf("pin_try_counter=%u\n", a->pin_try_counter);
    emv_session_free(p);

    // A card that goes quiet after the PPSE answer leaves everything else untouched.
    p = emv_session_alloc();
    emv_session_run(p, card, 34);
    printf("quiet_pan_len=%u\n", p->data->emv_application.pan_len);
    emv_session_free(p);
    return 0;
}
