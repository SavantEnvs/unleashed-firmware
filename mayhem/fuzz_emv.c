// libFuzzer harness for the EMV poller (lib/nfc/protocols/emv/emv_poller_i.c): the input is the
// byte script of everything an EMV card says during a read (PPSE, application, GPO, records, log):
// [length][reply] ... The TLV decoding of those card-controlled replies is what is fuzzed.
#include "emv_session.h"

int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    if(size == 0 || size > 4096) return -1;
    EmvPoller* p = emv_session_alloc();
    emv_session_run(p, data, size);
    emv_session_free(p);
    return 0;
}
