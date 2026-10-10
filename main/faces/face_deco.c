#include "face.h"
#include <math.h>
#define TAU 6.28318530718f

static void bg(canvas_t *c, const face_time_t *t) {
    c->font_style = 2;
    draw_ring(c, 233, 233, 211, 2, 0xb3975e);
    draw_ring(c, 233, 233, 205, 1, 0x6d6346);
    for (int i = 0; i < 12; i++) {
        float a = i / 12.f * TAU, s = sinf(a), co = cosf(a), x = 233 + 180 * s, y = 233 - 180 * co;
        draw_line(c, x - 7 * co, y - 7 * s, x + 19 * s, y - 19 * co, 2, 0xdac48c);
        draw_line(c, x + 7 * co, y + 7 * s, x + 19 * s, y - 19 * co, 2, 0xdac48c);
    }
    face_arabic(c, 145, 4, 0xe1cc96, true);
    draw_text(c, 233, 306, "DECO", 2, 0xad975e);
}
static void fg(canvas_t *c, const face_time_t *t) {
    face_hands(c, t, 98, 157, 7, 4, 0xd9bc7d);
    draw_hand(c, 233, 233, face_minute(t), 150, -35, 1, 0x263941);
    face_seconds(c, t, 192, 24, 1, 0xf0ead4);
    face_hub(c, 0xd9bc7d);
}
const clock_face_t face_deco = {"ART DECO", true, 0x101b22, NULL, bg, fg, NULL, false};
