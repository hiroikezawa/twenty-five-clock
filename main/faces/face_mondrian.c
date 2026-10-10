#include "face.h"
#include <math.h>
#define TAU 6.28318530718f

static void bg(canvas_t *c, const face_time_t *t) {
    for (int y = 102; y < 199; y++)
        draw_line(c, 105, y, 190, y, 1, 0xc63532);
    for (int y = 258; y < 356; y++)
        draw_line(c, 282, y, 352, y, 1, 0x255ba1);
    for (int y = 279; y < 337; y++)
        draw_line(c, 119, y, 179, y, 1, 0xe4be37);
    draw_line(c, 194, 59, 194, 408, 5, 0x24282a);
    draw_line(c, 67, 204, 405, 204, 5, 0x24282a);
    draw_line(c, 277, 65, 277, 405, 5, 0x24282a);
    draw_line(c, 61, 252, 410, 252, 5, 0x24282a);
    face_arabic(c, 184, 4, 0x24282a, true);
    draw_ticks(c, 12, 213, 7, 4, 0x24282a);
}
static void fg(canvas_t *c, const face_time_t *t) {
    face_hands(c, t, 101, 166, 13, 7, 0xf5f2e8);
    face_hands(c, t, 101, 166, 7, 3, 0x161b20);
    face_seconds(c, t, 194, 19, 2, 0xc63532);
    face_hub(c, 0x161b20);
}
const clock_face_t face_mondrian = {"MONDRIAN", false, 0xece9dd, NULL, bg, fg, NULL, false};
