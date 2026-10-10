#include "face.h"
#include <math.h>
const clock_face_t *const faces[FACE_COUNT] = {
    &face_bauhaus,    &face_swiss,       &face_pilot,      &face_diver,        &face_retro,
    &face_classic,    &face_chronograph, &face_sector,     &face_astronomical, &face_industrial,
    &face_minimal,    &face_radar,       &face_generative, &face_deco,         &face_mondrian,
    &face_orbit,      &face_compass,     &face_fan,        &face_blueprint,    &face_petals,
    &face_concentric, &face_station,     &face_ribbon,     &face_skeleton,     &face_typography};
float face_hour(const face_time_t *t) {
    return (t->local.tm_hour % 12 + t->local.tm_min / 60.f + t->second / 3600.f) / 12;
}
float face_minute(const face_time_t *t) {
    return (t->local.tm_min + t->second / 60.f) / 60;
}
void face_hands(canvas_t *c, const face_time_t *t, float hl, float ml, float hw, float mw,
                ink_t col) {
    draw_hand(c, 233, 233, face_hour(t), hl, 8, hw, col);
    draw_hand(c, 233, 233, face_minute(t), ml, 12, mw, col);
}
void face_seconds(canvas_t *c, const face_time_t *t, float len, float tail, float w, ink_t col) {
    draw_hand(c, 233, 233, t->second / 60, len, tail, w, col);
}
void face_hub(canvas_t *c, ink_t col) {
    draw_disc(c, 233, 233, 4, col);
}
void face_arabic(canvas_t *c, float r, int scale, ink_t col, bool quarters) {
    for (int i = 1; i <= 12; i++)
        if (!quarters || i % 3 == 0)
            draw_number(c, i, i / 12.f, r, scale, col);
}
double face_moon_phase(time_t utc) {
    /* Mean synodic month from 2000-01-06 18:14 UTC new moon; approximate. */
    double p = fmod((utc - 947182440.0) / 86400.0, 29.530588853) / 29.530588853;
    return p < 0 ? p + 1 : p;
}
