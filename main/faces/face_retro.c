#include "face.h"
static void bg(canvas_t *c, const face_time_t *t) {
    c->font_style = 1;
    draw_ring(c, 233, 233, 190, 23, 0x854a29);
    draw_ring(c, 233, 233, 211, 3, 0x482d24);
    face_arabic(c, 155, 4, 0x523425, true);
    draw_ticks(c, 12, 184, 12, 6, 0xe9cd95);
    draw_disc(c, 233, 310, 18, 0xdb662d);
    draw_text(c, 233, 112, "ORBIT", 2, 0x854a29);
}
static void fg(canvas_t *c, const face_time_t *t) {
    draw_hand(c, 233, 233, face_hour(t), 102, 21, 21, 0x563423);
    draw_hand(c, 233, 233, face_minute(t), 144, 25, 12, 0x985131);
    face_seconds(c, t, 184, 36, 3, 0xeb6b24);
    draw_disc(c, 233, 233, 9, 0xeb6b24);
}
const clock_face_t face_retro = {"1970S RETRO", true, 0xf0dfb6, NULL, bg, fg, NULL, false};
