#include "face.h"
#include <math.h>
#define TAU 6.28318530718f

static void bg(canvas_t *c, const face_time_t *t) {
    for (int i = 0; i < 12; i++) {
        draw_arc(c, 233, 233, 194, i / 12.f + .012f, i / 12.f + .065f, 8,
                 i % 3 == 0 ? 0xbda1cf : 0x574262);
        draw_arc(c, 233, 233, 178, i / 12.f + .025f, i / 12.f + .076f, 2, 0x7c588e);
    }
    face_arabic(c, 149, 4, 0xf0dbe9, true);
    draw_text(c, 233, 314, "RIBBON", 1, 0x9d799c);
}
static void fg(canvas_t *c, const face_time_t *t) {
    float h = face_hour(t), m = face_minute(t);
    draw_hand(c, 233, 233, h, 109, 20, 12, 0xd694b9);
    draw_hand(c, 233, 233, m, 165, 25, 7, 0xc8bce8);
    draw_hand(c, 233, 233, m, 151, -33, 2, 0x6a507e);
    face_seconds(c, t, 209, 0, 1, 0xf0d69c);
    face_hub(c, 0xf0d69c);
}
const clock_face_t face_ribbon = {"RIBBON", true, 0x120c1b, NULL, bg, fg, NULL, false};
