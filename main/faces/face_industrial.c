#include "face.h"
static void bg(canvas_t *c, const face_time_t *t) {
    c->font_style = 3;
    draw_ring(c, 233, 233, 215, 5, 0x6f797b);
    draw_ring(c, 233, 233, 204, 1, 0x343e42);
    draw_ticks(c, 60, 192, 9, 1, 0xb0babe);
    face_arabic(c, 161, 3, 0xdbdfd7, true);
    draw_ticks(c, 12, 192, 19, 4, 0xcbd3ce);
    int x[] = {104, 362, 104, 362}, y[] = {104, 104, 362, 362};
    for (int i = 0; i < 4; i++) {
        draw_disc(c, x[i], y[i], 8, 0x687073);
        draw_line(c, x[i] - 4, y[i] + 3, x[i] + 4, y[i] - 3, 2, 0x22282b);
    }
    draw_text(c, 233, 294, "TYPE 13", 2, 0xa0acaa);
    draw_text(c, 233, 315, "QUARTZ / JST", 1, 0x71807e);
}
static void fg(canvas_t *c, const face_time_t *t) {
    face_hands(c, t, 108, 150, 12, 8, 0xe0e5d9);
    face_seconds(c, t, 187, 38, 3, 0xf9bf37);
    draw_disc(c, 233, 233, 10, 0x747b75);
    draw_disc(c, 233, 233, 4, 0x22282b);
}
const clock_face_t face_industrial = {"INDUSTRIAL", false, 0x161c1f, NULL, bg, fg, NULL, false};
