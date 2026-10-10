#pragma once
#include <stdbool.h>
#include "esp_err.h"
typedef struct {
    unsigned char day, evening, night;
    bool sweep, auto_face;
} clock_settings_t;
extern clock_settings_t settings;
void settings_init(void);
esp_err_t settings_save(void);
bool settings_wifi(char ssid[33], char password[65]);
esp_err_t settings_save_wifi(const char *ssid, const char *password);
esp_err_t settings_change_ssid(const char *ssid);
int settings_brightness(int hour);
