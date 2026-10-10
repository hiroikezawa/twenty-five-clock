#include "face.h"
#include <math.h>
#define TAU 6.28318530718f

static void bg(canvas_t *c, const face_time_t *t) {
    for (int i = 0; i < 12; i++) {
        float a = i * TAU / 12;
        float x = 233 + 165 * sinf(a), y = 233 - 165 * cosf(a);
        for (int j = 0; j < 20; j++) {
            float u = j * TAU / 20, v = (j + 1) * TAU / 20;
            draw_line(c, x + 13 * cosf(u) * cosf(a) + 29 * sinf(u) * sinf(a),
                      y + 13 * cosf(u) * sinf(a) - 29 * sinf(u) * cosf(a),
                      x + 13 * cosf(v) * cosf(a) + 29 * sinf(v) * sinf(a),
                      y + 13 * cosf(v) * sinf(a) - 29 * sinf(v) * cosf(a), 1.5, 0x779a76);
        }
        draw_disc(c, 233 + 211 * sinf(a), 233 - 211 * cosf(a), 2, 0xcbb4a1);
    }
    face_arabic(c, 124, 3, 0xd4c8b4, true);
}
static void fg(canvas_t *c, const face_time_t *t) {
    face_hands(c, t, 94, 152, 6, 3, 0xd4c8b4);
    face_seconds(c, t, 194, 12, 1, 0xc28c82);
    face_hub(c, 0xc28c82);
}
const clock_face_t face_petals = {"BOTANICAL", true, 0x080e0c, NULL, bg, fg, NULL, false};
