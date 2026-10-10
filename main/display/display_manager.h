#pragma once
#include "draw.h"
void display_init(void);
canvas_t *display_static_begin(int dx, int dy, ink_t background);
void display_static_end(void);
canvas_t *display_frame_begin(void);
void display_frame_end(void);
void display_brightness(int percent);
unsigned display_last_pixels(void);
bool display_rotation(float radians);
void display_touch_unrotate(uint16_t *x, uint16_t *y);
