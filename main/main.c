#include "board.h"
#include "peripherals.h"
#include "display_manager.h"
#include "settings.h"
#include "rtc_manager.h"
#include "ntp_manager.h"
#include "touch_manager.h"
#include "clock_manager.h"
#include "esp_flash.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "render_benchmark.h"
#include "gravity.h"
#include "usb_time.h"
#ifndef CONFIG_ESPTOOLPY_FLASHSIZE_16MB
#error "non-C 1.75 requires 16 MiB flash"
#endif
_Static_assert(CONFIG_CLOCK_DAY_START < CONFIG_CLOCK_EVENING_START &&
                   CONFIG_CLOCK_EVENING_START < CONFIG_CLOCK_NIGHT_START,
               "brightness periods must be increasing");
void app_main(void) {
    uint32_t size = 0;
    ESP_ERROR_CHECK(esp_flash_get_size(NULL, &size));
    ESP_LOGI("Boot", "Waveshare ESP32-S3-Touch-AMOLED-1.75 NON-C | Flash %lu MiB",
             (unsigned long)(size >> 20));
    settings_init();
    ESP_ERROR_CHECK(board_i2c_init());
    peripherals_init();
    display_init();
    rtc_boot_time();
    usb_time_start();
    gravity_start();
    touch_init();
#ifdef CONFIG_CLOCK_SELF_TEST
    render_benchmark();
#endif
    ntp_manager_start();
    clock_manager_run();
}
