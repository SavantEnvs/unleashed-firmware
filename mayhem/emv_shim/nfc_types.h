// The few NFC stack types emv.h / emv_poller_i.h name, so the EMV poller compiles without the firmware's NFC stack.
#pragma once
#include <furi.h>
#include <lib/toolbox/bit_buffer.h>

typedef struct Iso14443_4aData Iso14443_4aData;
typedef struct Iso14443_4aPoller Iso14443_4aPoller;
typedef struct NfcDeviceBase NfcDeviceBase;
typedef struct FlipperFormat FlipperFormat;
typedef int NfcDeviceNameType;

typedef enum {
    Iso14443_4aErrorNone,
    Iso14443_4aErrorNotPresent,
    Iso14443_4aErrorTimeout,
    Iso14443_4aErrorProtocol,
} Iso14443_4aError;

typedef struct {
    int protocol;
    void* instance;
    void* event_data;
} NfcGenericEvent;
typedef int (*NfcGenericCallback)(NfcGenericEvent event, void* context);

Iso14443_4aError iso14443_4a_poller_send_block_pwt_ext(
    Iso14443_4aPoller* instance,
    const BitBuffer* tx_buffer,
    BitBuffer* rx_buffer);
