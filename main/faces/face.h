#pragma once
#include "draw.h"
#include "face_count.h"
#include <time.h>
typedef struct {
    struct tm local;
    time_t utc;
    double second;
} face_time_t;
typedef struct {
    const char *name;
    bool sweep;
    ink_t background;
    void (*init)(void);
    void (*draw_static)(canvas_t *, const face_time_t *);
    void (*draw_dynamic)(canvas_t *, const face_time_t *);
    void (*deinit)(void);
    bool minute_cache; /* Rebuild static artwork each minute (generative only). */
} clock_face_t;
extern const clock_face_t face_bauhaus, face_swiss, face_pilot, face_diver, face_retro,
    face_classic, face_chronograph, face_sector, face_astronomical, face_industrial, face_minimal,
    face_radar, face_generative, face_deco, face_mondrian, face_orbit, face_compass, face_fan,
    face_blueprint, face_petals, face_concentric, face_station, face_ribbon, face_skeleton,
    face_typography;
extern const clock_face_t *const faces[FACE_COUNT];
float face_hour(const face_time_t *t);
float face_minute(const face_time_t *t);
void face_hands(canvas_t *c, const face_time_t *t, float hl, float ml, float hw, float mw,
                ink_t color);
void face_seconds(canvas_t *c, const face_time_t *t, float len, float tail, float width,
                  ink_t color);
void face_hub(canvas_t *c, ink_t color);
void face_arabic(canvas_t *c, float radius, int scale, ink_t color, bool quarters);
double face_moon_phase(time_t utc);
