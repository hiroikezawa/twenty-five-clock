#include "face.h"
#include <math.h>
#define TAU 6.28318530718f

static void bg(canvas_t *c, const face_time_t *t) {
    c->font_style = 0;
    draw_ring(c, 233, 233, 211, 4, 0x274044);
    draw_ticks(c, 60, 202, 7, 2, 0x315053);
    face_arabic(c, 161, 4, 0x20363b, false);
    draw_line(c, 187, 280, 279, 280, 2, 0x768f8b);
    draw_text(c, 233, 302, "STATION", 2, 0x43615e);
}
static void fg(canvas_t *c, const face_time_t *t) {
    face_hands(c, t, 101, 148, 11, 6, 0x20363b);
    face_seconds(c, t, 189, 26, 2, 0xcf6a32);
    float a = t->second * TAU / 60;
    draw_line(c, 233 + 189 * sinf(a) - 7 * cosf(a), 233 - 189 * cosf(a) - 7 * sinf(a),
              233 + 189 * sinf(a) + 7 * cosf(a), 233 - 189 * cosf(a) + 7 * sinf(a), 3, 0xcf6a32);
    face_hub(c, 0x20363b);
}
const clock_face_t face_station = {"STATION", false, 0xe8ede7, NULL, bg, fg, NULL, false};
