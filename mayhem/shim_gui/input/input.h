#pragma once
// Host stand-in for applications/services/input/input.h (the real one pulls in furi_hal_resources.h).
#include <stdint.h>

typedef enum {
    InputKeyUp,
    InputKeyDown,
    InputKeyRight,
    InputKeyLeft,
    InputKeyOk,
    InputKeyBack,
    InputKeyMAX,
} InputKey;

typedef enum {
    InputTypePress,
    InputTypeRelease,
    InputTypeShort,
    InputTypeLong,
    InputTypeRepeat,
    InputTypeMAX,
} InputType;

typedef struct {
    union {
        uint32_t sequence;
        struct {
            uint8_t sequence_source : 2;
            uint32_t sequence_counter : 30;
        };
    };
    InputKey key;
    InputType type;
} InputEvent;
