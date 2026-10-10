#include "face.h"
static void bg(canvas_t *c, const face_time_t *t) {
    draw_ticks(c, 12, 205, 10, 1, 0xeeeeee);
    face_arabic(c, 174, 3, 0xffffff, true);
}
static void fg(canvas_t *c, const face_time_t *t) {
    face_hands(c, t, 104, 153, 2, 2, 0xffffff);
    face_seconds(c, t, 170, 22, 1.2f, 0xe43c39);
    face_hub(c, 0xe43c39);
}
const clock_face_t face_bauhaus = {"BAUHAUS", true, 0, NULL, bg, fg, NULL, false};
