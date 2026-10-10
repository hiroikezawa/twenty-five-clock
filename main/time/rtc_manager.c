#include "rtc_manager.h"
#include "peripherals.h"
#include "esp_log.h"
#include <sys/time.h>
#include <stdlib.h>
#include <stdatomic.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
static i2c_master_dev_handle_t dev;
static atomic_bool valid, system_valid;
static SemaphoreHandle_t lock;
static int dec(uint8_t b) {
    return (b >> 4) * 10 + (b & 15);
}
static uint8_t bcd(int d) {
    return (d / 10) * 16 + d % 10;
}
bool rtc_valid(void) {
    return valid;
}
bool clock_time_valid(void) {
    return system_valid;
}
esp_err_t rtc_manager_init(void) {
    lock = xSemaphoreCreateMutex();
    assert(lock);
    return device_open(0x51, &dev);
}
static esp_err_t read_locked(time_t *utc) {
    uint8_t ctrl, r[7];
    valid = false;
    esp_err_t e = device_read(dev, 0, &ctrl, 1);
    if (e != ESP_OK)
        return e;
    /* Reject STOP, 12-hour mode, oscillator-stop and invalid BCD/calendar. */
    if (ctrl & 0x22)
        return ESP_ERR_INVALID_STATE;
    e = device_read(dev, 4, r, 7);
    if (e != ESP_OK)
        return e;
    if (r[0] & 0x80)
        return ESP_ERR_INVALID_STATE;
    const uint8_t masks[] = {0x7f, 0x7f, 0x3f, 0x3f, 0x07, 0x1f, 0xff};
    for (int i = 0; i < 7; i++) {
        r[i] &= masks[i];
        if ((r[i] & 15) > 9 || (r[i] >> 4) > 9)
            return ESP_ERR_INVALID_RESPONSE;
    }
    struct tm t = {.tm_year = 100 + dec(r[6]),
                   .tm_mon = dec(r[5]) - 1,
                   .tm_mday = dec(r[3]),
                   .tm_hour = dec(r[2]),
                   .tm_min = dec(r[1]),
                   .tm_sec = dec(r[0]),
                   .tm_isdst = 0};
    if (t.tm_year < 124 || t.tm_mon < 0 || t.tm_mon > 11 || t.tm_mday < 1 || t.tm_mday > 31 ||
        t.tm_hour > 23 || t.tm_min > 59 || t.tm_sec > 59 || r[4] > 6)
        return ESP_ERR_INVALID_RESPONSE;
    struct tm expected = t;
    /* RTC holds JST, matching the Waveshare local wall-clock example. */
    *utc = mktime(&t);
    if (t.tm_mday != expected.tm_mday || t.tm_mon != expected.tm_mon)
        return ESP_ERR_INVALID_RESPONSE;
    valid = true;
    return ESP_OK;
}
static esp_err_t write_locked(time_t utc) {
    struct tm t;
    localtime_r(&utc, &t);
    if (t.tm_year < 100 || t.tm_year > 199)
        return ESP_ERR_INVALID_ARG;
    uint8_t ctrl;
    esp_err_t e = device_read(dev, 0, &ctrl, 1);
    if (e != ESP_OK)
        return e;
    ctrl = (ctrl & ~0xa2) | 0x20; /* stop while updating, 24h, normal mode */
    if ((e = device_write(dev, 0, &ctrl, 1)) != ESP_OK)
        return e;
    uint8_t r[] = {bcd(t.tm_sec), bcd(t.tm_min),     bcd(t.tm_hour),      bcd(t.tm_mday),
                   t.tm_wday,     bcd(t.tm_mon + 1), bcd(t.tm_year - 100)};
    e = device_write(dev, 4, r, 7);
    ctrl &= ~0x20;
    esp_err_t resume = device_write(dev, 0, &ctrl, 1);
    valid = (e == ESP_OK && resume == ESP_OK);
    return e != ESP_OK ? e : resume;
}
esp_err_t rtc_read(time_t *utc) {
    xSemaphoreTake(lock, portMAX_DELAY);
    esp_err_t e = read_locked(utc);
    xSemaphoreGive(lock);
    return e;
}
esp_err_t rtc_write(time_t utc) {
    system_valid = true; /* Called with validated NTP/USB time; RTC may still fail. */
    xSemaphoreTake(lock, portMAX_DELAY);
    esp_err_t e = write_locked(utc);
    xSemaphoreGive(lock);
    return e;
}
void rtc_boot_time(void) {
    setenv("TZ", "JST-9", 1);
    tzset();
    ESP_ERROR_CHECK(rtc_manager_init());
    time_t t;
    if (rtc_read(&t) == ESP_OK) {
        system_valid = true;
        ESP_LOGI("RTC", "valid JST time restored");
    } else {
        /* Explicitly marked unsynchronized until NTP/manual setup succeeds. */
        struct tm fallback = {.tm_year = 126, .tm_mon = 0, .tm_mday = 1};
        t = mktime(&fallback);
        ESP_LOGW("RTC", "invalid/unpowered RTC; provisional 2026-01-01, waiting for setup/NTP");
    }
    struct timeval tv = {.tv_sec = t};
    settimeofday(&tv, NULL);
}
