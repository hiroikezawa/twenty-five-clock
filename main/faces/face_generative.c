#include "face.h"
#include "schedule.h"
#include <math.h>
static void bg(canvas_t *c, const face_time_t *t) {
    uint32_t seed = clock_date_seed(t->local.tm_year + 1900, t->local.tm_mon + 1, t->local.tm_mday);
    int minute = t->local.tm_hour * 60 + t->local.tm_min;
    int petals = 5 + seed % 5;
    /* Date selects the motif; each minute rotates it by only half a degree.
       Radius changes continuously with minute count instead of jumping noise. */
    float phase = (seed >> 8) % 360 / 360.f + minute / 720.f;
    for (int i = 0; i < petals; i++) {
        float a = phase + i / (float)petals;
        float r = 144 + 22 * sinf(i * 6.2831853f / petals + minute * 0.04f + seed % 17);
        draw_arc(c, 233, 233, r, a, a + 0.65f / petals, 1.3f, i & 1 ? 0x366c86 : 0x673f83);
        float b = a * 6.2831853f;
        draw_disc(c, 233 + sinf(b) * r, 233 - cosf(b) * r, 3, 0x7ca6bd);
        draw_line(c, 233 + sinf(b) * 85, 233 - cosf(b) * 85, 233 + sinf(b + 0.4f) * r,
                  233 - cosf(b + 0.4f) * r, 1, 0x252c49);
    }
    draw_ticks(c, 12, 207, 8, 2, 0xb6c4de);
}
static void fg(canvas_t *c, const face_time_t *t) {
    face_hands(c, t, 96, 160, 6, 3, 0xf0eefa);
    float a = t->second * 6.2831853f / 60;
    draw_disc(c, 233 + sinf(a) * 211, 233 - cosf(a) * 211, 4, 0xd3a2ff);
    face_hub(c, 0xd3a2ff);
}
const clock_face_t face_generative = {"GENERATIVE", true, 0, NULL, bg, fg, NULL, true};
