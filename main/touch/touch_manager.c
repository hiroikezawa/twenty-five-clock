#include "touch_manager.h"
#include "board.h"
#include "esp_log.h"
#include "gesture.h"
#include "display_manager.h"
static esp_lcd_touch_handle_t touch;
static bool is_pressed;
bool touch_is_pressed(void) {
    return is_pressed;
}

static int64_t last_poll;
void touch_init(void) {
    esp_err_t e = board_touch_new(&touch);
    if (e != ESP_OK) {
        touch = NULL;
        ESP_LOGW("Touch", "CST9217 unavailable: %s", esp_err_to_name(e));
    }
}
touch_event_t touch_poll(int64_t now) {
    touch_event_t out = {0};
    if (!touch || now - last_poll < 10000)
        return out;
    last_poll = now;
    if (esp_lcd_touch_read_data(touch) != ESP_OK)
        return out;
    esp_lcd_touch_point_data_t point = {0};
    uint8_t n = 0;
    if (esp_lcd_touch_get_data(touch, &point, &n, 1) != ESP_OK)
        return out;
    uint16_t x = point.x, y = point.y;
    display_touch_unrotate(&x, &y);
    bool pressed = n > 0;
    is_pressed = pressed;
    return gesture_update(pressed, x, y, now);
}
