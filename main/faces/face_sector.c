#include "face.h"
static void bg(canvas_t *c, const face_time_t *t) {
    c->font_style = 1;
    draw_ring(c, 233, 233, 210, 2, 0x454947);
    draw_ring(c, 233, 233, 146, 2, 0x454947);
    draw_ticks(c, 60, 204, 6, 1, 0x626860);
    draw_ticks(c, 12, 146, 13, 3, 0x454947);
    face_arabic(c, 176, 4, 0x303b36, true);
    for (int i = 0; i < 4; i++)
        draw_hand(c, 233, 233, i / 4.f, 136, -20, 1, 0x9a9c8e);
    draw_text(c, 233, 307, "SECTOR", 1, 0x697569);
}
static void fg(canvas_t *c, const face_time_t *t) {
    face_hands(c, t, 105, 154, 6, 3, 0x303b36);
    face_seconds(c, t, 198, 22, 1, 0xb05c39);
    face_hub(c, 0xb05c39);
}
const clock_face_t face_sector = {"SECTOR", true, 0xdddccc, NULL, bg, fg, NULL, false};
