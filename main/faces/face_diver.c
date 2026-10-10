#include "face.h"
static void bg(canvas_t *c, const face_time_t *t) {
    draw_ring(c, 233, 233, 193, 2, 0x456061);
    draw_ticks(c, 60, 215, 6, 1, 0x82948d);
    for (int i = 1; i < 12; i++)
        draw_number(c, i * 5, i / 12.f, 203, 1, 0xbad3c3);
    draw_ticks(c, 12, 172, 22, 13, 0xd9eed0);
    draw_line(c, 226, 60, 240, 60, 8, 0xd9eed0);
    draw_text(c, 233, 295, "DEEP", 2, 0x7ca398);
    draw_text(c, 233, 317, "60 MIN", 1, 0x7ca398);
}
static void fg(canvas_t *c, const face_time_t *t) {
    face_hands(c, t, 97, 144, 18, 12, 0xd9eed0);
    draw_hand(c, 233, 233, face_hour(t), 90, 0, 4, 0x34534b);
    face_seconds(c, t, 179, 25, 2, 0xf2c667);
    face_hub(c, 0xf2c667);
}
const clock_face_t face_diver = {"DIVER", false, 0x020a0b, NULL, bg, fg, NULL, false};
