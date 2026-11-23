#pragma once
#include <io/pad.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    bool cross;
    bool circle;
    bool triangle;
    bool square;
    bool up;
    bool down;
    bool left;
    bool right;
    bool l1;
    bool r1;
    bool start;
    bool select;
} PadButtons;

void pad_init(void);
void pad_update(void);

// Pressed only this frame (edge triggered)
bool pad_just_pressed(uint32_t btn_mask);
// Held down
bool pad_is_held(uint32_t btn_mask);

#ifdef __cplusplus
}
#endif
