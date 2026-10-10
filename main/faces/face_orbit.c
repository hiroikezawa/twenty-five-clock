#include "face.h"
#include <math.h>
#define TAU 6.28318530718f

static void bg(canvas_t *c, const face_time_t *t) {
    draw_ring(c, 233, 233, 112, 1, 0x243249);
    draw_ring(c, 233, 233, 163, 1, 0x304666);
    draw_arc(c, 222, 233, 191, .06, .82, 2, 0x23324c);
    for (int i = 0; i < 12; i++) {
        float a = i * TAU / 12;
        draw_disc(c, 233 + 198 * sinf(a), 233 - 198 * cosf(a), i % 3 == 0 ? 6 : 3, 0x8bb3dd);
    }
    face_arabic(c, 181, 2, 0xabcde4, true);
}
static void fg(canvas_t *c, const face_time_t *t) {
    float h = face_hour(t) * TAU, m = face_minute(t) * TAU;
    face_hands(c, t, 112, 163, 5, 2, 0xb8d8f2);
    draw_ring(c, 233 + 112 * sinf(h), 233 - 112 * cosf(h), 9, 3, 0xe5bc76);
    draw_disc(c, 233 + 163 * sinf(m), 233 - 163 * cosf(m), 5, 0xb8d8f2);
    float a = t->second * TAU / 60;
    draw_disc(c, 233 + 212 * sinf(a), 233 - 212 * cosf(a), 3, 0xe5bc76);
    face_hub(c, 0xe5bc76);
}
const clock_face_t face_orbit = {"ORBITAL", true, 0x030710, NULL, bg, fg, NULL, false};
