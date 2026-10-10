#pragma once
#include <time.h>
#include <stdbool.h>
#include "esp_err.h"
esp_err_t rtc_manager_init(void);
esp_err_t rtc_read(time_t *utc);
esp_err_t rtc_write(time_t utc);
bool rtc_valid(void);
bool clock_time_valid(void);
void rtc_boot_time(void);
