#include "face.h"
#include <math.h>
#define TAU 6.28318530718f

static void bg(canvas_t *c, const face_time_t *t) {
    c->font_style = 3;
    for (int v = -168; v <= 168; v += 24) {
        float r = sqrtf(190 * 190 - v * v);
        draw_line(c, 233 + v, 233 - r, 233 + v, 233 + r, 1, 0x193b59);
        draw_line(c, 233 - r, 233 + v, 233 + r, 233 + v, 1, 0x193b59);
    }
    draw_ring(c, 233, 233, 207, 1, 0x76b1c9);
    draw_ticks(c, 60, 207, 5, 1, 0x93c8df);
    face_arabic(c, 177, 3, 0xd3eced, false);
    draw_ring(c, 233, 233, 125, 1, 0x426a81);
    draw_text(c, 233, 308, "SCALE 1:1", 1, 0x8baab8);
}
static void fg(canvas_t *c, const face_time_t *t) {
    face_hands(c, t, 110, 155, 5, 3, 0xe4f7f3);
    face_seconds(c, t, 197, 18, 1, 0xe8b968);
    draw_ring(c, 233, 233, 10, 1, 0xe4f7f3);
}
const clock_face_t face_blueprint = {"BLUEPRINT", false, 0x0b2845, NULL, bg, fg, NULL, false};
