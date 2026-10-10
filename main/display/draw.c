#include "draw.h"
#include "font_data.h"
#include <math.h>
#include <string.h>
#include <stdio.h>
#define TAU 6.28318530718f
static uint16_t rgb(ink_t c) {
    return ((c >> 8) & 0xf800) | ((c >> 5) & 0x7e0) | ((c >> 3) & 31);
}
void draw_clear(canvas_t *c, ink_t col) {
    uint16_t p = rgb(col);
    for (int i = 0; i < SCREEN * SCREEN; i++)
        c->pixels[i] = p;
    if (c->tracking)
        for (int i = 0; i < TILE_COUNT; i++)
            c->dirty[i] = 0x7fff;
}
static void raw_pixel(canvas_t *c, int x, int y, ink_t col, float a) {
    x += c->dx;
    y += c->dy;
    if (x < 0 || y < 0 || x >= SCREEN || y >= SCREEN || a <= 0)
        return;
    uint16_t *p = &c->pixels[y * SCREEN + x];
    if (a >= 0.99f)
        *p = rgb(col);
    else {
        unsigned r = (*p >> 11) * 255 / 31, g = ((*p >> 5) & 63) * 255 / 63,
                 b = (*p & 31) * 255 / 31;
        r += (int)(((col >> 16) & 255) - (int)r) * a;
        g += (int)(((col >> 8) & 255) - (int)g) * a;
        b += (int)((col & 255) - (int)b) * a;
        *p = rgb((r << 16) | (g << 8) | b);
    }
    if (c->tracking)
        c->dirty[y / TILE] |= 1u << (x / TILE);
}
static void transform(const canvas_t *c, float *x, float *y) {
    if (!c->rotated)
        return;
    float px = *x + c->dx - 233, py = *y + c->dy - 233;
    *x = 233 + c->rc * px - c->rs * py - c->dx;
    *y = 233 + c->rs * px + c->rc * py - c->dy;
}
void draw_pixel(canvas_t *c, int x, int y, ink_t col, float a) {
    float xx = x, yy = y;
    transform(c, &xx, &yy);
    raw_pixel(c, lroundf(xx), lroundf(yy), col, a);
}
/* Inverse bilinear sampling once per static/orientation change. Dynamic
   hands rotate their vector endpoints before AA rasterization instead. */
void draw_rotate_cache(canvas_t *out, const canvas_t *src, float radians) {
    float co = cosf(radians), si = sinf(radians);
    for (int y = 0; y < SCREEN; y++)
        for (int x = 0; x < SCREEN; x++) {
            float sx = 233 + co * (x - 233) + si * (y - 233),
                  sy = 233 - si * (x - 233) + co * (y - 233);
            int ix = floorf(sx), iy = floorf(sy);
            uint16_t p = src->pixels[0];
            if (ix >= 0 && iy >= 0 && ix < SCREEN - 1 && iy < SCREEN - 1) {
                unsigned fx = (sx - ix) * 256, fy = (sy - iy) * 256;
                unsigned weights[] = {(256 - fx) * (256 - fy), fx * (256 - fy), (256 - fx) * fy,
                                      fx * fy};
                unsigned red = 0, green = 0, blue = 0;
                const uint16_t *base = src->pixels + iy * SCREEN + ix;
                for (int k = 0; k < 4; k++) {
                    unsigned sample = base[(k & 1) + ((k & 2) ? SCREEN : 0)];
                    red += (sample >> 11) * weights[k];
                    green += ((sample >> 5) & 63) * weights[k];
                    blue += (sample & 31) * weights[k];
                }
                p = ((red >> 16) << 11) | ((green >> 16) << 5) | (blue >> 16);
            } else if (ix >= 0 && iy >= 0 && ix < SCREEN && iy < SCREEN)
                p = src->pixels[iy * SCREEN + ix];
            out->pixels[y * SCREEN + x] = p;
        }
}
void draw_line(canvas_t *c, float x0, float y0, float x1, float y1, float width, ink_t col) {
    transform(c, &x0, &y0);
    transform(c, &x1, &y1);
    float vx = x1 - x0, vy = y1 - y0, len2 = vx * vx + vy * vy, r = width / 2, ext = r + 1;
    float inv_len = len2 > 0 ? 1.f / sqrtf(len2) : 0;
    float inv_vy = fabsf(vy) > 0.01f ? 1.f / vy : 0;
    float inner = fmaxf(0, r - 0.5f), inner2 = inner * inner;
    int ymin = fmaxf(-c->dy, floorf(fminf(y0, y1) - ext));
    int ymax = fminf(SCREEN - 1 - c->dy, ceilf(fmaxf(y0, y1) + ext));
    for (int y = ymin; y <= ymax; y++) {
        float lo = fminf(x0, x1) - ext, hi = fmaxf(x0, x1) + ext;
        if (fabsf(vy) > 0.01f) {
            float a = fmaxf(0, fminf(1, (y - ext - y0) * inv_vy)),
                  b = fmaxf(0, fminf(1, (y + ext - y0) * inv_vy));
            lo = fminf(x0 + a * vx, x0 + b * vx) - ext;
            hi = fmaxf(x0 + a * vx, x0 + b * vx) + ext;
        }
        int xmin = fmaxf(-c->dx, floorf(lo)), xmax = fminf(SCREEN - 1 - c->dx, ceilf(hi));
        for (int x = xmin; x <= xmax; x++) {
            float px = x - x0, py = y - y0, dot = px * vx + py * vy;
            if (len2 > 0 && dot >= 0 && dot <= len2) {
                /* Exact perpendicular distance in the shaft: one square root
                   per line, not thousands per frame. Rounded caps below. */
                float distance = fabsf(px * vy - py * vx) * inv_len;
                if (distance < r + 0.5f)
                    raw_pixel(c, x, y, col, r + 0.5f - distance);
            } else {
                float dx = dot > 0 ? x - x1 : px, dy = dot > 0 ? y - y1 : py;
                float d2 = dx * dx + dy * dy;
                if (d2 < ext * ext)
                    raw_pixel(c, x, y, col, d2 <= inner2 ? 1 : r + 0.5f - sqrtf(d2));
            }
        }
    }
}
void draw_disc(canvas_t *c, float x, float y, float r, ink_t col) {
    transform(c, &x, &y);
    for (int j = floorf(y - r - 1); j <= ceilf(y + r + 1); j++)
        for (int i = floorf(x - r - 1); i <= ceilf(x + r + 1); i++) {
            float dx = i - x, dy = j - y, d = dx * dx + dy * dy;
            if (d < (r + 1) * (r + 1))
                raw_pixel(c, i, j, col, fminf(1, r + 0.5f - sqrtf(d)));
        }
}
void draw_arc(canvas_t *c, float x, float y, float r, float from, float to, float w, ink_t col) {
    int n = ceilf(fabsf(to - from) * r * TAU / 3);
    if (n < 1)
        n = 1;
    float a = from * TAU, px = x + sinf(a) * r, py = y - cosf(a) * r;
    for (int i = 1; i <= n; i++) {
        a = (from + (to - from) * i / n) * TAU;
        float nx = x + sinf(a) * r, ny = y - cosf(a) * r;
        draw_line(c, px, py, nx, ny, w, col);
        px = nx;
        py = ny;
    }
}
void draw_ring(canvas_t *c, float x, float y, float r, float w, ink_t col) {
    draw_arc(c, x, y, r, 0, 1, w, col);
}
void draw_hand(canvas_t *c, float x, float y, float t, float len, float tail, float w, ink_t col) {
    float s = sinf(t * TAU), co = cosf(t * TAU);
    draw_line(c, x - s * tail, y + co * tail, x + s * len, y - co * len, w, col);
}
void draw_ticks(canvas_t *c, int n, float r, float len, float w, ink_t col) {
    for (int i = 0; i < n; i++) {
        float a = i * TAU / n, s = sinf(a), co = cosf(a);
        draw_line(c, 233 + s * (r - len), 233 - co * (r - len), 233 + s * r, 233 - co * r, w, col);
    }
}
/* Original compact 5x7 glyphs; no external font/licensing dependency. */
static const uint8_t glyphs[][7] = {
    {14, 17, 19, 21, 25, 17, 14}, {4, 12, 4, 4, 4, 4, 14},      {14, 17, 1, 2, 4, 8, 31},
    {30, 1, 1, 14, 1, 1, 30},     {2, 6, 10, 18, 31, 2, 2},     {31, 16, 16, 30, 1, 1, 30},
    {14, 16, 16, 30, 17, 17, 14}, {31, 1, 2, 4, 8, 8, 8},       {14, 17, 17, 14, 17, 17, 14},
    {14, 17, 17, 15, 1, 1, 14},   {14, 17, 17, 31, 17, 17, 17}, {30, 17, 17, 30, 17, 17, 30},
    {14, 17, 16, 16, 16, 17, 14}, {30, 17, 17, 17, 17, 17, 30}, {31, 16, 16, 30, 16, 16, 31},
    {31, 16, 16, 30, 16, 16, 16}, {14, 17, 16, 23, 17, 17, 15}, {17, 17, 17, 31, 17, 17, 17},
    {14, 4, 4, 4, 4, 4, 14},      {7, 2, 2, 2, 18, 18, 12},     {17, 18, 20, 24, 20, 18, 17},
    {16, 16, 16, 16, 16, 16, 31}, {17, 27, 21, 21, 17, 17, 17}, {17, 25, 25, 21, 19, 19, 17},
    {14, 17, 17, 17, 17, 17, 14}, {30, 17, 17, 30, 16, 16, 16}, {14, 17, 17, 17, 21, 18, 13},
    {30, 17, 17, 30, 20, 18, 17}, {15, 16, 16, 14, 1, 1, 30},   {31, 4, 4, 4, 4, 4, 4},
    {17, 17, 17, 17, 17, 17, 14}, {17, 17, 17, 17, 17, 10, 4},  {17, 17, 17, 21, 21, 21, 10},
    {17, 17, 10, 4, 10, 17, 17},  {17, 17, 10, 4, 4, 4, 4},     {31, 1, 2, 4, 8, 16, 31},
    {0, 4, 4, 0, 4, 4, 0},        {0, 0, 0, 31, 0, 0, 0},       {0, 0, 4, 14, 4, 0, 0},
    {0, 0, 0, 0, 0, 6, 6},        {1, 2, 2, 4, 8, 8, 16},       {17, 2, 4, 8, 16, 0, 17}};
static void bitmap_text(canvas_t *c, float cx, float cy, const char *text, int scale, ink_t col) {
    int len = strlen(text), start = lroundf(cx - (len * 6 - 1) * scale / 2.0f),
        y0 = lroundf(cy - 7 * scale / 2.0f);
    for (int n = 0; n < len; n++) {
        char ch = text[n];
        if (ch >= 'a' && ch <= 'z')
            ch -= 32;
        int g = ch >= '0' && ch <= '9'   ? ch - '0'
                : ch >= 'A' && ch <= 'Z' ? ch - 'A' + 10
                : ch == ':'              ? 36
                : ch == '-'              ? 37
                : ch == '+'              ? 38
                : ch == '.'              ? 39
                : ch == '/'              ? 40
                : ch == '%'              ? 41
                                         : -1;
        if (g < 0)
            continue;
        for (int y = 0; y < 7; y++)
            for (int x = 0; x < 5; x++)
                if (glyphs[g][y] & (1 << (4 - x)))
                    for (int sy = 0; sy < scale; sy++)
                        for (int sx = 0; sx < scale; sx++)
                            draw_pixel(c, start + n * 6 * scale + x * scale + sx,
                                       y0 + y * scale + sy, col, 1);
    }
}
void draw_text(canvas_t *c, float cx, float cy, const char *text, int scale, ink_t col) {
    if (c->font_style == 3) {
        bitmap_text(c, cx, cy, text, scale, col);
        return;
    }
    if (scale < 1)
        scale = 1;
    if (scale > 4)
        scale = 4;
    unsigned style = c->font_style > 2 ? 0 : c->font_style;
    const font_record_t *font = &font_records[style * 4 + scale - 1];
    int width = 0;
    for (const char *p = text; *p; p++) {
        char ch = *p;
        if (ch >= 'a' && ch <= 'z')
            ch -= 32;
        const char *found = strchr(font_chars, ch);
        if (found)
            width += font_glyphs[font->first + found - font_chars].advance;
    }
    int x = lroundf(cx - width / 2.f), y = lroundf(cy - font->height / 2.f);
    for (const char *p = text; *p; p++) {
        char ch = *p;
        if (ch >= 'a' && ch <= 'z')
            ch -= 32;
        const char *found = strchr(font_chars, ch);
        if (!found)
            continue;
        const font_glyph_t *g = &font_glyphs[font->first + found - font_chars];
        for (int j = 0; j < g->height; j++)
            for (int i = 0; i < g->width; i++) {
                int k = j * g->width + i;
                unsigned a = font_alpha[g->offset + k / 2];
                a = k & 1 ? a & 15 : a >> 4;
                if (a)
                    draw_pixel(c, x + g->x + i, y + g->y + j, col, a / 15.f);
            }
        x += g->advance;
    }
}
void draw_restore(canvas_t *c, const canvas_t *background, const uint16_t dirty[TILE_COUNT]) {
    for (int ty = 0; ty < TILE_COUNT; ty++)
        for (int tx = 0; tx < TILE_COUNT; tx++)
            if (dirty[ty] & (1u << tx)) {
                int x = tx * TILE, w = x + TILE > SCREEN ? SCREEN - x : TILE;
                for (int y = ty * TILE; y < (ty + 1) * TILE && y < SCREEN; y++)
                    memcpy(c->pixels + y * SCREEN + x, background->pixels + y * SCREEN + x, w * 2);
            }
}
void draw_number(canvas_t *c, int n, float t, float r, int size, ink_t col) {
    char s[8];
    snprintf(s, sizeof(s), "%d", n);
    draw_text(c, 233 + sinf(t * TAU) * r, 233 - cosf(t * TAU) * r, s, size, col);
}
