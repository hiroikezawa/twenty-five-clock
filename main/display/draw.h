#pragma once
#include <stdint.h>
#include <stdbool.h>
#define SCREEN 466
#define SAFE_RADIUS 218
#define TILE 32
#define TILE_COUNT 15
typedef uint32_t ink_t;
typedef struct {
    uint16_t *pixels;
    uint16_t dirty[TILE_COUNT];
    int dx, dy;
    bool tracking;
    unsigned font_style;
    bool rotated;
    float rc, rs;
} canvas_t;
void draw_rotate_cache(canvas_t *out, const canvas_t *source, float radians);
void draw_restore(canvas_t *c, const canvas_t *background, const uint16_t dirty[TILE_COUNT]);
void draw_clear(canvas_t *c, ink_t color);
void draw_pixel(canvas_t *c, int x, int y, ink_t color, float coverage);
void draw_line(canvas_t *c, float x0, float y0, float x1, float y1, float width, ink_t color);
void draw_disc(canvas_t *c, float x, float y, float radius, ink_t color);
void draw_ring(canvas_t *c, float x, float y, float radius, float width, ink_t color);
void draw_arc(canvas_t *c, float x, float y, float radius, float from, float to, float width,
              ink_t color);
void draw_text(canvas_t *c, float x, float y, const char *text, int scale, ink_t color);
void draw_hand(canvas_t *c, float x, float y, float turns, float length, float tail, float width,
               ink_t color);
void draw_ticks(canvas_t *c, int count, float r, float length, float width, ink_t color);
void draw_number(canvas_t *c, int n, float turns, float radius, int size, ink_t color);
