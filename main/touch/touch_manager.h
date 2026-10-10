#pragma once
#include <stdbool.h>
bool touch_is_pressed(void);
#include <stdint.h>
typedef enum {
    TOUCH_NONE,
    TOUCH_TAP,
    TOUCH_DOUBLE,
    TOUCH_LEFT,
    TOUCH_RIGHT,
    TOUCH_LONG
} touch_kind_t;
typedef struct {
    touch_kind_t kind;
    int x, y;
} touch_event_t;
void touch_init(void);
touch_event_t touch_poll(int64_t now_us);
