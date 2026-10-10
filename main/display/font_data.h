#pragma once
#include <stdint.h>
typedef struct {
    uint32_t offset;
    uint8_t width, height;
    int8_t x, y;
    uint8_t advance;
} font_glyph_t;
typedef struct {
    uint16_t first;
    uint8_t height;
} font_record_t;
extern const unsigned char font_alpha[];
extern const font_glyph_t font_glyphs[];
extern const font_record_t font_records[];
extern const char font_chars[];
