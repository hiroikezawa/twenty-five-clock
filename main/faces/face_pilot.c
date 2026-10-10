#include "face.h"
static void bg(canvas_t *c, const face_time_t *t) {
    draw_ticks(c, 60, 212, 7, 1, 0xd5dfe0);
    draw_ticks(c, 12, 212, 15, 4, 0xffffff);
    for (int i = 1; i < 12; i++)
        draw_number(c, i, i / 12.f, 172, 4, 0xffffff);
    draw_line(c, 222, 74, 233, 52, 4, 0xffffff);
    draw_line(c, 233, 52, 244, 74, 4, 0xffffff);
    draw_line(c, 222, 74, 244, 74, 4, 0xffffff);
    draw_disc(c, 211, 64, 3, 0xffffff);
    draw_disc(c, 255, 64, 3, 0xffffff);
    draw_text(c, 233, 286, "JST", 2, 0x879399);
}
static void fg(canvas_t *c, const face_time_t *t) {
    face_hands(c, t, 110, 150, 13, 9, 0xf5f5dd);
    face_seconds(c, t, 195, 27, 1, 0xffffff);
    face_hub(c, 0x343e44);
}
const clock_face_t face_pilot = {"PILOT", true, 0x030608, NULL, bg, fg, NULL, false};
