#include "face.h"
#include <math.h>
#define TAU 6.28318530718f

static void bg(canvas_t *c, const face_time_t *t) {
    for (int i = 0; i < 48; i++) {
        float a = i * TAU / 48;
        float r = 140 + (i % 4) * 15;
        draw_line(c, 233 + 50 * sinf(a), 233 - 50 * cosf(a), 233 + r * sinf(a), 233 - r * cosf(a),
                  i % 4 == 0 ? 3 : 1, i % 4 == 0 ? 0xaea48f : 0xd0c6b3);
    }
    draw_disc(c, 233, 233, 64, 0xeee6d9);
    face_arabic(c, 199, 3, 0x5a5a4b, true);
    draw_ticks(c, 12, 177, 6, 3, 0x5a5a4b);
}
static void fg(canvas_t *c, const face_time_t *t) {
    face_hands(c, t, 105, 158, 8, 4, 0x444c45);
    face_seconds(c, t, 183, 27, 1.5, 0xaa5344);
    face_hub(c, 0xaa5344);
}
const clock_face_t face_fan = {"PAPER FAN", true, 0xeee6d9, NULL, bg, fg, NULL, false};
