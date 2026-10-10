#include "face.h"
#include <math.h>
static void bg(canvas_t *c, const face_time_t *t) {
    draw_ticks(c, 60, 211, 9, 3, 0x101010);
    draw_ticks(c, 12, 210, 29, 10, 0x101010);
}
static void fg(canvas_t *c, const face_time_t *t) {
    face_hands(c, t, 117, 175, 15, 11, 0x101010);
    face_seconds(c, t, 157, 38, 3, 0xde272a);
    float a = t->second * 6.2831853f / 60;
    draw_disc(c, 233 + sinf(a) * 151, 233 - cosf(a) * 151, 10, 0xde272a);
    face_hub(c, 0xde272a);
}
const clock_face_t face_swiss = {"SWISS RAILWAY", false, 0xf2f1ea, NULL, bg, fg, NULL, false};
