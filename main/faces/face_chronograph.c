#include "face.h"
#include <math.h>
#include <stdio.h>
static void sub(canvas_t *c, int x, int y, const char *label, int ticks) {
    draw_ring(c, x, y, 46, 1, 0x607078);
    for (int i = 0; i < ticks; i++)
        draw_hand(c, x, y, i / (float)ticks, 42, -37, 1, 0x94a2a8);
    draw_text(c, x, y + 21, label, 1, 0xa5b2b9);
}
static void bg(canvas_t *c, const face_time_t *t) {
    draw_ticks(c, 120, 215, 4, 1, 0x778992);
    draw_ticks(c, 12, 209, 17, 4, 0xe4e8e8);
    sub(c, 326, 233, "DATE", 31);
    char date[4];
    snprintf(date, sizeof(date), "%d", t->local.tm_mday);
    draw_text(c, 326, 213, date, 1, 0xffffff);
    sub(c, 233, 326, "SEC", 12);
    sub(c, 140, 233, "DAY", 7);
    draw_text(c, 233, 120, "CHRONO", 2, 0x9faeb5);
    const char *days[] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
    draw_text(c, 140, 213, days[t->local.tm_wday], 1, 0xffffff);
}
static void fg(canvas_t *c, const face_time_t *t) {
    draw_hand(c, 326, 233, (t->local.tm_mday - 1) / 31.f, 34, 5, 2, 0xffb75b);
    draw_hand(c, 233, 326, t->second / 60, 34, 5, 1.5, 0xffb75b);
    draw_hand(c, 140, 233, t->local.tm_wday / 7.f, 34, 5, 2, 0xffb75b);
    face_hands(c, t, 111, 177, 7, 4, 0xe8edef);
    face_seconds(c, t, 199, 33, 1.5, 0xf89b41);
    face_hub(c, 0xf89b41);
}
const clock_face_t face_chronograph = {"CHRONOGRAPH", true, 0x06090b, NULL, bg, fg, NULL, false};
