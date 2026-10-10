#include "face.h"
#include <math.h>
#define TAU 6.28318530718f

static void bg(canvas_t *c, const face_time_t *t) {
    const int xs[] = {155, 311, 233}, ys[] = {185, 185, 314}, rs[] = {51, 51, 44};
    for (int k = 0; k < 3; k++) {
        draw_ring(c, xs[k], ys[k], rs[k], 2, 0x5c6468);
        draw_ring(c, xs[k], ys[k], rs[k] - 9, 1, 0x313b40);
        for (int j = 0; j < 12; j++)
            draw_hand(c, xs[k], ys[k], j / 12.f, rs[k] + 4, -rs[k] + 3, 3, 0x5c6468);
        draw_ring(c, xs[k], ys[k], 9, 3, 0x9c8564);
    }
    draw_line(c, 156, 174, 233, 312, 6, 0x3a4448);
    draw_line(c, 311, 174, 233, 312, 6, 0x3a4448);
    draw_ring(c, 233, 233, 208, 2, 0xb19f7b);
    draw_ticks(c, 12, 198, 17, 5, 0xe1d3af);
    face_arabic(c, 173, 2, 0xe1d3af, true);
}
static void fg(canvas_t *c, const face_time_t *t) {
    face_hands(c, t, 110, 165, 10, 7, 0xe3d9bb);
    face_hands(c, t, 101, 155, 4, 2, 0x303b40);
    face_seconds(c, t, 191, 29, 1, 0x98bace);
    face_hub(c, 0xe3d9bb);
}
const clock_face_t face_skeleton = {"SKELETON", true, 0x0e1216, NULL, bg, fg, NULL, false};
