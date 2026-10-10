#include "face.h"
static void bg(canvas_t *c, const face_time_t *t) {
    for (int r = 50; r <= 200; r += 50)
        draw_ring(c, 233, 233, r, 1, 0x123b2b);
    draw_line(c, 33, 233, 433, 233, 1, 0x123b2b);
    draw_line(c, 233, 33, 233, 433, 1, 0x123b2b);
    draw_ticks(c, 60, 214, 4, 1, 0x267b4f);
    face_arabic(c, 183, 2, 0x367b57, true);
}
static void fg(canvas_t *c, const face_time_t *t) {
    /* Four opaque spokes, no full-screen alpha trail. */
    const ink_t trail[] = {0x062318, 0x0d4228, 0x16713b, 0x67ed83};
    for (int i = 0; i < 4; i++)
        draw_hand(c, 233, 233, t->second / 60 - (3 - i) * 0.008f, 204, 0, i == 3 ? 1.5 : 2,
                  trail[i]);
    face_hands(c, t, 94, 143, 5, 3, 0x83ae96);
    face_hub(c, 0xb5ffbe);
}
const clock_face_t face_radar = {"RADAR", true, 0, NULL, bg, fg, NULL, false};
