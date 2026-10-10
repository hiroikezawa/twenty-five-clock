#include "display_manager.h"
#include "board.h"
#include "esp_heap_caps.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_io.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include <string.h>
#include <math.h>
static canvas_t background, frame, level_background;
static uint16_t pending[TILE_COUNT], previous[TILE_COUNT];
static uint16_t *dma;
static esp_lcd_panel_handle_t panel;
static SemaphoreHandle_t complete;
static unsigned last_pixels;
static float rotation, rc = 1, rs;
static bool rotated;
static void rebuild_rotation(void);
bool display_rotation(float radians) {
    float delta = remainderf(radians - rotation, 6.28318530718f);
    if (fabsf(delta) < 0.02618f)
        return false; /* 1.5 degree noise dead band */
    rotation = radians;
    rc = cosf(rotation);
    rs = sinf(rotation);
    rotated = true;
    frame.rotated = true;
    frame.rc = rc;
    frame.rs = rs;
    rebuild_rotation();
    return true;
}
void display_touch_unrotate(uint16_t *x, uint16_t *y) {
    float px = *x - 233, py = *y - 233;
    *x = (uint16_t)fmaxf(0, fminf(465, lroundf(233 + rc * px + rs * py)));
    *y = (uint16_t)fmaxf(0, fminf(465, lroundf(233 - rs * px + rc * py)));
}
static bool IRAM_ATTR sent(esp_lcd_panel_io_handle_t io, esp_lcd_panel_io_event_data_t *event,
                           void *ctx) {
    BaseType_t wake = pdFALSE;
    xSemaphoreGiveFromISR(complete, &wake);
    /* IDF 5.5's SPI panel IO discards the boolean returned by this callback.
       Explicitly request the ISR-exit switch, otherwise each small rectangle
       can wait until the next 1 ms tick even after DMA has completed. */
    if (wake == pdTRUE)
        portYIELD_FROM_ISR();
    return false;
}
void display_init(void) {
    complete = xSemaphoreCreateBinary();
    assert(complete);
    background.pixels = heap_caps_malloc(SCREEN * SCREEN * 2, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    frame.pixels = heap_caps_malloc(SCREEN * SCREEN * 2, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    level_background.pixels =
        heap_caps_malloc(SCREEN * SCREEN * 2, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    dma = heap_caps_malloc(SCREEN * TILE * 2, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    assert(background.pixels && frame.pixels && dma && level_background.pixels);
    frame.tracking = true;
    esp_lcd_panel_io_handle_t io;
    ESP_ERROR_CHECK(board_display_new(SCREEN * TILE * 2, &panel, &io));
    esp_lcd_panel_io_callbacks_t cb = {.on_color_trans_done = sent};
    ESP_ERROR_CHECK(esp_lcd_panel_io_register_event_callbacks(io, &cb, NULL));
    display_brightness(8);
    display_static_begin(0, 0, 0);
    display_static_end();
    display_frame_begin();
    display_frame_end();
}
canvas_t *display_static_begin(int dx, int dy, ink_t col) {
    background.dx = frame.dx = dx;
    background.dy = frame.dy = dy;
    background.font_style = frame.font_style = 0;
    draw_clear(&background, col);
    return &background;
}
static void rebuild_rotation(void) {
    if (rotated)
        draw_rotate_cache(&level_background, &background, rotation);
    else
        memcpy(level_background.pixels, background.pixels, SCREEN * SCREEN * 2);
    memcpy(frame.pixels, level_background.pixels, SCREEN * SCREEN * 2);
    frame.font_style = background.font_style;
    memset(previous, 0, sizeof(previous));
    for (int i = 0; i < TILE_COUNT; i++)
        pending[i] = 0x7fff;
}
void display_static_end(void) {
    rebuild_rotation();
}
canvas_t *display_frame_begin(void) {
    draw_restore(&frame, &level_background, previous);
    for (int ty = 0; ty < TILE_COUNT; ty++) {
        pending[ty] |= previous[ty];
    }
    memset(frame.dirty, 0, sizeof(frame.dirty));
    return &frame;
}
void display_frame_end(void) {
    last_pixels = 0;
    uint16_t dirty[TILE_COUNT];
    for (int ty = 0; ty < TILE_COUNT; ty++) {
        dirty[ty] = pending[ty] | frame.dirty[ty];
        previous[ty] = frame.dirty[ty];
        pending[ty] = 0;
    }
    for (int ty = 0; ty < TILE_COUNT; ty++) {
        for (int tx = 0; tx < TILE_COUNT; tx++)
            if (dirty[ty] & (1u << tx)) {
                /* Merge adjacent dirty tiles in BOTH axes. For the upper-left
                   dirty tile choose the largest fully dirty rectangle that
                   fits the fixed DMA buffer. No clean pixels or malloc. */
                int best_w = 1, best_h = 1, best_area = 1;
                for (int tw = 1; tx + tw <= TILE_COUNT; tw++) {
                    uint16_t bits = ((1u << tw) - 1u) << tx;
                    if ((dirty[ty] & bits) != bits)
                        break;
                    for (int th = 1; ty + th <= TILE_COUNT; th++) {
                        if ((dirty[ty + th - 1] & bits) != bits)
                            break;
                        int w = ((tx + tw) * TILE > SCREEN ? SCREEN : (tx + tw) * TILE) - tx * TILE;
                        int h = ((ty + th) * TILE > SCREEN ? SCREEN : (ty + th) * TILE) - ty * TILE;
                        if (w * h > SCREEN * TILE)
                            break;
                        if (tw * th > best_area) {
                            best_area = tw * th;
                            best_w = tw;
                            best_h = th;
                        }
                    }
                }
                uint16_t bits = ((1u << best_w) - 1u) << tx;
                for (int row = ty; row < ty + best_h; row++)
                    dirty[row] &= ~bits;
                int x = tx * TILE, xe = (tx + best_w) * TILE;
                if (xe > SCREEN)
                    xe = SCREEN;
                int y = ty * TILE, ye = (ty + best_h) * TILE;
                if (ye > SCREEN)
                    ye = SCREEN;
                int w = xe - x;
                for (int row = y; row < ye; row++)
                    for (int j = 0; j < w; j++) {
                        uint16_t p = frame.pixels[row * SCREEN + x + j];
                        dma[(row - y) * w + j] = (p >> 8) | (p << 8);
                    }
                ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(panel, x, y, xe, ye, dma));
                /* No DMA buffer reuse until the queued color transaction completes. */
                if (xSemaphoreTake(complete, pdMS_TO_TICKS(1000)) != pdTRUE)
                    abort();
                last_pixels += w * (ye - y);
                tx += best_w - 1;
            }
    }
}
void display_brightness(int percent) {
    ESP_ERROR_CHECK(board_brightness_set(percent));
}
unsigned display_last_pixels(void) {
    return last_pixels;
}
