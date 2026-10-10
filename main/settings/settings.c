#include "settings.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "sdkconfig.h"
#include <string.h>
clock_settings_t settings;
static nvs_handle_t nvs;
void settings_init(void) {
    esp_err_t err = nvs_flash_init();
    /* Never silently erase user credentials on NVS failure. */
    ESP_ERROR_CHECK(err);
    ESP_ERROR_CHECK(nvs_open("clock", NVS_READWRITE, &nvs));
    settings =
        (clock_settings_t){CONFIG_CLOCK_DAY, CONFIG_CLOCK_EVENING, CONFIG_CLOCK_NIGHT, true, true};
    uint8_t v;
    if (nvs_get_u8(nvs, "day", &v) == ESP_OK && v >= 1 && v <= 100)
        settings.day = v;
    if (nvs_get_u8(nvs, "evening", &v) == ESP_OK && v >= 1 && v <= 100)
        settings.evening = v;
    if (nvs_get_u8(nvs, "night", &v) == ESP_OK && v >= 1 && v <= 100)
        settings.night = v;
    if (nvs_get_u8(nvs, "sweep", &v) == ESP_OK)
        settings.sweep = !!v;
    if (nvs_get_u8(nvs, "auto", &v) == ESP_OK)
        settings.auto_face = !!v;
    /* One-time migration requested for the brighter clock revision. Later
       user adjustments continue to persist unchanged, including low values. */
    if (nvs_get_u8(nvs, "bright_rev", &v) != ESP_OK || v < 1) {
        settings.day = 85;
        settings.evening = 65;
        settings.night = 30;
        ESP_ERROR_CHECK(nvs_set_u8(nvs, "bright_rev", 1));
        ESP_ERROR_CHECK(settings_save());
    }
}
esp_err_t settings_save(void) {
    esp_err_t e;
#define SAVE(k, v)                                                                                 \
    do {                                                                                           \
        if ((e = nvs_set_u8(nvs, k, v)) != ESP_OK)                                                 \
            return e;                                                                              \
    } while (0)
    SAVE("day", settings.day);
    SAVE("evening", settings.evening);
    SAVE("night", settings.night);
    SAVE("sweep", settings.sweep);
    SAVE("auto", settings.auto_face);
    return nvs_commit(nvs);
}
bool settings_wifi(char ssid[33], char password[65]) {
    size_t s = 33, p = 65;
    return nvs_get_str(nvs, "ssid", ssid, &s) == ESP_OK &&
           nvs_get_str(nvs, "password", password, &p) == ESP_OK && ssid[0];
}
esp_err_t settings_save_wifi(const char *ssid, const char *password) {
    if (!ssid[0] || strlen(ssid) > 32 || strlen(password) > 63 ||
        (password[0] && strlen(password) < 8))
        return ESP_ERR_INVALID_ARG;
    esp_err_t e = nvs_set_str(nvs, "ssid", ssid);
    if (e != ESP_OK)
        return e;
    e = nvs_set_str(nvs, "password", password);
    return e == ESP_OK ? nvs_commit(nvs) : e;
}
esp_err_t settings_change_ssid(const char *ssid) {
    if (!ssid[0] || strlen(ssid) > 32)
        return ESP_ERR_INVALID_ARG;
    /* Preserve the existing password without reading or exporting it. */
    size_t length = 0;
    esp_err_t e = nvs_get_str(nvs, "password", NULL, &length);
    if (e != ESP_OK)
        return e;
    e = nvs_set_str(nvs, "ssid", ssid);
    return e == ESP_OK ? nvs_commit(nvs) : e;
}
int settings_brightness(int hour) {
    return hour >= CONFIG_CLOCK_DAY_START && hour < CONFIG_CLOCK_EVENING_START ? settings.day
           : hour >= CONFIG_CLOCK_EVENING_START && hour < CONFIG_CLOCK_NIGHT_START
               ? settings.evening
               : settings.night;
}
