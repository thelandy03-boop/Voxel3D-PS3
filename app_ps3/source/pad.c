#include "pad.h"
#include <string.h>

static padInfo pad_info;
static padData pad_data;
static uint32_t last_buttons = 0;
static uint32_t curr_buttons = 0;

void pad_init(void) {
    ioPadInit(7);
}

void pad_update(void) {
    last_buttons = curr_buttons;
    curr_buttons = 0;

    ioPadGetInfo(&pad_info);
    for (int i = 0; i < MAX_PADS; i++) {
        if (pad_info.status[i]) {
            ioPadGetData(i, &pad_data);
            if (pad_data.len > 0) {
                // Map pad buttons bitmask
                curr_buttons = (pad_data.button[2] << 8) | pad_data.button[3];
                break;
            }
        }
    }
}

bool pad_just_pressed(uint32_t btn_mask) {
    return ((curr_buttons & btn_mask) && !(last_buttons & btn_mask));
}

bool pad_is_held(uint32_t btn_mask) {
    return (curr_buttons & btn_mask) != 0;
}
