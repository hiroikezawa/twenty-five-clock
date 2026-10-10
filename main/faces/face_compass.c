#include "face.h"
#include <math.h>
#define TAU 6.28318530718f

static void bg(canvas_t *c, const face_time_t *t) {
    draw_ticks(c, 60, 213, 5, 1, 0x9dbfc5);
    draw_ring(c, 233, 233, 157, 1, 0x33525a);
    for (int i = 0; i < 8; i++) {
        float a = i * TAU / 8, x = 233 + 116 * sinf(a), y = 233 - 116 * cosf(a);
        draw_line(c, 233, 233, x, y, i % 2 == 0 ? 6 : 2, 0x29474e);
        draw_line(c, x, y, 233 + 42 * sinf(a + .22f), 233 - 42 * cosf(a + .22f), 1, 0x6d8783);
    }
    face_arabic(c, 184, 4, 0xe0d8b2, true);
    draw_text(c, 233, 324, "MARINER", 1, 0x9bafab);
}
static void fg(canvas_t *c, const face_time_t *t) {
    face_hands(c, t, 106, 160, 8, 4, 0xe8dbad);
    float a = face_minute(t) * TAU, x = 233 + 160 * sinf(a), y = 233 - 160 * cosf(a);
    draw_line(c, x, y, 233 + 136 * sinf(a - .07f), 233 - 136 * cosf(a - .07f), 3, 0xe8dbad);
    draw_line(c, x, y, 233 + 136 * sinf(a + .07f), 233 - 136 * cosf(a + .07f), 3, 0xe8dbad);
    face_seconds(c, t, 204, 20, 1, 0xc87b58);
    face_hub(c, 0xc87b58);
}
const clock_face_t face_compass = {"MARINER", false, 0x071e2a, NULL, bg, fg, NULL, false};
