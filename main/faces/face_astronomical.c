#include "face.h"
#include <math.h>
#include <stdio.h>
static void bg(canvas_t *c, const face_time_t *t) {
    draw_ring(c, 233, 233, 213, 1, 0x87704a);
    draw_ring(c, 233, 233, 177, 1, 0x87704a);
    face_arabic(c, 196, 2, 0xc5ac78, false);
    draw_ticks(c, 12, 143, 6, 2, 0xc5ac78);
    double phase = face_moon_phase(t->utc), angle = phase * 6.28318530718;
    draw_disc(c, 233, 144, 22, 0x263149);
    /* Orthographic sphere illuminated by the mean lunar phase. */
    for (int y = -21; y <= 21; y++)
        for (int x = -21; x <= 21; x++) {
            float nx = x / 22.f, ny = y / 22.f, z2 = 1 - nx * nx - ny * ny;
            if (z2 > 0 && nx * sin(angle) - sqrtf(z2) * cos(angle) > 0)
                draw_pixel(c, 233 + x, 144 + y, 0xe7dfba, 1);
        }
    char date[24];
    const char *days[] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
    snprintf(date, sizeof(date), "%02d-%02d %s", t->local.tm_mon + 1, t->local.tm_mday,
             days[t->local.tm_wday]);
    draw_text(c, 233, 323, date, 2, 0xc5ac78);
    snprintf(date, sizeof(date), "MOON %.1f", phase * 29.530588853);
    draw_text(c, 233, 347, date, 1, 0x7e90a3);
}
static void fg(canvas_t *c, const face_time_t *t) {
    face_hands(c, t, 83, 126, 4, 2, 0xe4d2ad);
    face_seconds(c, t, 139, 18, 1, 0x83a7c2);
    face_hub(c, 0xe4d2ad);
}
const clock_face_t face_astronomical = {"ASTRONOMICAL", true, 0x070b18, NULL, bg, fg, NULL, false};
