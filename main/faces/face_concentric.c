#include "face.h"
#include <math.h>
#define TAU 6.28318530718f

static void bg(canvas_t *c, const face_time_t *t) {
    draw_ring(c, 233, 233, 103, 2, 0x394250);
    draw_ring(c, 233, 233, 163, 2, 0x394250);
    draw_ring(c, 233, 233, 211, 1, 0x394250);
    face_arabic(c, 190, 3, 0xd6dbe1, false);
    draw_ticks(c, 60, 163, 4, 1, 0x748394);
    draw_text(c, 233, 293, "H / M / S", 1, 0x778594);
}
static void fg(canvas_t *c, const face_time_t *t) {
    float h = face_hour(t), m = face_minute(t), s = t->second / 60;
    draw_arc(c, 233, 233, 103, h - .032f, h + .032f, 7, 0xe6aa63);
    draw_arc(c, 233, 233, 163, m - .020f, m + .020f, 4, 0xaac6e4);
    draw_arc(c, 233, 233, 211, s - .010f, s + .010f, 3, 0xca7185);
    draw_hand(c, 233, 233, h, 103, 0, 5, 0xe6aa63);
    draw_hand(c, 233, 233, m, 163, 0, 2, 0xaac6e4);
    face_hub(c, 0xdde5ed);
}
const clock_face_t face_concentric = {"CONCENTRIC", true, 0x08090d, NULL, bg, fg, NULL, false};
