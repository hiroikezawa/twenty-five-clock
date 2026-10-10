#include "face.h"
#include <math.h>
static void bg(canvas_t *c, const face_time_t *t) {
    for (int i = 0; i < 12; i++)
        draw_disc(c, 233 + sinf(i * 6.2831853f / 12) * 185, 233 - cosf(i * 6.2831853f / 12) * 185,
                  2, 0x9ca8ad);
}
static void fg(canvas_t *c, const face_time_t *t) {
    draw_hand(c, 233, 233, face_hour(t), 93, 0, 4, 0xf0f6f9);
    draw_hand(c, 233, 233, face_minute(t), 145, 0, 2, 0xb3c5d1);
    float a = t->second * 6.2831853f / 60;
    draw_disc(c, 233 + sinf(a) * 210, 233 - cosf(a) * 210, 3, 0x98dfe5);
}
const clock_face_t face_minimal = {"MINIMAL DOT", true, 0, NULL, bg, fg, NULL, false};
