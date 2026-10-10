#include "usb_time.h"
#include "rtc_manager.h"
#include "settings.h"
#include "esp_system.h"
#include "driver/usb_serial_jtag.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
static void receive(void *arg) {
    char line[64];
    unsigned n = 0;
    bool overflow = false;
    for (;;) {
        char ch;
        if (usb_serial_jtag_read_bytes(&ch, 1, pdMS_TO_TICKS(100)) != 1)
            continue;
        if (ch == '\n') {
            line[n] = 0;
            if (!overflow && !strncmp(line, "SSID ", 5)) {
                esp_err_t e = settings_change_ssid(line + 5);
                ESP_LOGI("USB_WIFI", "SSID update %s; existing password preserved", esp_err_to_name(e));
                if (e == ESP_OK) {
                    vTaskDelay(pdMS_TO_TICKS(1000));
                    esp_restart();
                }
            }
            if (!overflow && !strncmp(line, "TIME ", 5)) {
                char *end;
                long long epoch = strtoll(line + 5, &end, 10);
                if (*end == 0 && epoch >= 1704067200LL && epoch < 4102444800LL) {
                    struct timeval tv = {.tv_sec = epoch};
                    settimeofday(&tv, NULL);
                    esp_err_t e = rtc_write(tv.tv_sec);
                    ESP_LOGI("USB_TIME", "TIME OK %lld RTC %s", epoch, esp_err_to_name(e));
                }
            }
            n = 0;
            overflow = false;
        } else if (ch != '\r') {
            if (n < sizeof(line) - 1)
                line[n++] = ch;
            else
                overflow = true;
        }
    }
}
void usb_time_start(void) {
    usb_serial_jtag_driver_config_t cfg = {.tx_buffer_size = 1024, .rx_buffer_size = 256};
    ESP_ERROR_CHECK(usb_serial_jtag_driver_install(&cfg));
    if (xTaskCreate(receive, "usb_time", 3072, NULL, 2, NULL) != pdPASS)
        abort();
}
