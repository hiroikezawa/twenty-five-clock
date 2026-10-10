#include "gesture.h"
#include <stdlib.h>
static bool down, long_sent, pending, moved;
static int sx, sy, lx, ly, tx, ty;
static int64_t start, last_tap;
void gesture_reset(void) {
    down = long_sent = pending = moved = false;
}
touch_event_t gesture_update(bool pressed, int x, int y, int64_t now) {
    touch_event_t out = {0};
    if (pressed && x < 466 && y < 466) {
        lx = x;
        ly = y;
        if (!down) {
            down = true;
            sx = x;
            sy = y;
            start = now;
            long_sent = false;
            moved = false;
        }
        if (abs(lx - sx) > 18 || abs(ly - sy) > 18)
            moved = true;
        if (now - start >= 700000 && !moved && !long_sent) {
            long_sent = true;
            pending = false;
            out = (touch_event_t){TOUCH_LONG, lx, ly};
        }
    } else if (down) {
        down = false;
        if (!long_sent) {
            if (abs(lx - sx) >= 50 && abs(lx - sx) > abs(ly - sy)) {
                pending = false;
                out = (touch_event_t){lx < sx ? TOUCH_LEFT : TOUCH_RIGHT, lx, ly};
            } else if (!moved && now - start < 700000) {
                if (pending && now - last_tap < 330000 && abs(lx - tx) < 35 && abs(ly - ty) < 35) {
                    pending = false;
                    out = (touch_event_t){TOUCH_DOUBLE, lx, ly};
                } else {
                    pending = true;
                    last_tap = now;
                    tx = lx;
                    ty = ly;
                }
            }
        }
    }
    if (out.kind == TOUCH_NONE && pending && !down && now - last_tap >= 330000) {
        pending = false;
        out = (touch_event_t){TOUCH_TAP, tx, ty};
    }
    return out;
}
