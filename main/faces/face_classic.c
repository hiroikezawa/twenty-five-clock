#include "face.h"
#include <math.h>
#include <stdio.h>
static void bg(canvas_t *c, const face_time_t *t) {
    c->font_style = 2;
    const char *roman[] = {"XII", "I",   "II",   "III", "IV", "V",
                           "VI",  "VII", "VIII", "IX",  "X",  "XI"};
    draw_ticks(c, 60, 211, 5, 0.8f, 0xbaa878);
    draw_ring(c, 233, 233, 199, 0.8f, 0x766743);
    for (int i = 0; i < 12; i++)
        draw_text(c, 233 + sinf(i * 6.2831853f / 12) * 173, 233 - cosf(i * 6.2831853f / 12) * 173,
                  roman[i], 2, 0xe5d8b3);
    char date[12];
    snprintf(date, sizeof(date), "%02d", t->local.tm_mday);
    draw_text(c, 233, 342, date, 2, 0xe5d8b3);
    draw_line(c, 218, 328, 248, 328, 1, 0x766743);
    draw_line(c, 218, 356, 248, 356, 1, 0x766743);
}
static void fg(canvas_t *c, const face_time_t *t) {
    face_hands(c, t, 122, 169, 3, 2, 0xe4d5a9);
    face_seconds(c, t, 185, 36, 0.9f, 0x889caf);
    face_hub(c, 0xe4d5a9);
}
const clock_face_t face_classic = {"CLASSIC", true, 0x060d1a, NULL, bg, fg, NULL, false};
