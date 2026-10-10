#include "face.h"
#include <math.h>
#define TAU 6.28318530718f

static void bg(canvas_t *c, const face_time_t *t) {
    c->font_style = 3;
    draw_text(c, 233, 91, "12", 7, 0xf4f0df);
    draw_text(c, 379, 233, "3", 7, 0xf4f0df);
    draw_text(c, 233, 374, "6", 7, 0xf4f0df);
    draw_text(c, 86, 233, "9", 7, 0xf4f0df);
    for (int i = 0; i < 12; i++)
        if (i % 3)
            draw_hand(c, 233, 233, i / 12.f, 192, -175, 4, 0xb4b7ab);
    draw_text(c, 233, 303, "TYPE", 1, 0x7e837d);
}
static void fg(canvas_t *c, const face_time_t *t) {
    draw_hand(c, 233, 233, face_hour(t), 91, 9, 14, 0xe2c251);
    draw_hand(c, 233, 233, face_minute(t), 148, 12, 5, 0xf4f0df);
    float a = t->second * TAU / 60;
    draw_disc(c, 233 + 212 * sinf(a), 233 - 212 * cosf(a), 4, 0xe2c251);
    face_hub(c, 0xe2c251);
}
const clock_face_t face_typography = {"TYPOGRAPHY", false, 0x080909, NULL, bg, fg, NULL, false};
