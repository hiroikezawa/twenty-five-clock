#include "gravity.h"
#include "peripherals.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <math.h>
static portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
static float angle;
static i2c_master_dev_handle_t imu;
float gravity_angle(void) {
    portENTER_CRITICAL(&mux);
    float result = angle;
    portEXIT_CRITICAL(&mux);
    return result;
}
static void sample_task(void *arg) {
    float fx = 0, fy = 0;
    unsigned samples = 0;
    for (;;) {
        uint8_t r[6];
        if (device_read(imu, 0x35, r, sizeof(r)) == ESP_OK) {
            float x = (int16_t)(r[0] | r[1] << 8) / 16384.f;
            float y = (int16_t)(r[2] | r[3] << 8) / 16384.f;
            float z = (int16_t)(r[4] | r[5] << 8) / 16384.f;
            float norm = x * x + y * y + z * z;
            /* Reject shaking/free fall and hold orientation when face-up. */
            if (norm > .65f && norm < 1.4f) {
                fx += .15f * (x - fx);
                fy += .15f * (y - fy);
                if (fx * fx + fy * fy > .09f) {
                    /* Official non-C Gravitysphere workerLoop maps screen
                       gravity as (-accelY, +accelX). Rotated logical DOWN
                       is (-sin(angle), cos(angle)). No zero calibration:
                       removing the standing gravity would lose vertical. */
                    float target = atan2f(fy, fx);
                    portENTER_CRITICAL(&mux);
                    angle = target;
                    portEXIT_CRITICAL(&mux);
                }
            }
            if (samples++ % 1500 == 0)
                ESP_LOGI("Gravity", "accel %.3f %.3f %.3f g | angle %.1f deg", x, y, z,
                         gravity_angle() * 57.29578f);
        }
        vTaskDelay(pdMS_TO_TICKS(40));
    }
}
void gravity_start(void) {
    uint8_t id = 0;
    if (device_open(0x6b, &imu) != ESP_OK)
        return;
    if (device_read(imu, 0, &id, 1) != ESP_OK || id != 5) {
        ESP_LOGW("Gravity", "QMI8658 unavailable; orientation held");
        i2c_master_bus_rm_device(imu);
        return;
    }
    /* Official SensorLib CTRL1 auto increment, CTRL2 2g/125Hz,
       CTRL5 LPF mode 0, CTRL7 accelerometer only. Gyro stays off. */
    const uint8_t regs[] = {8, 2, 3, 6, 8}, vals[] = {0, 0x40, 6, 1, 1};
    for (unsigned i = 0; i < sizeof(regs); i++)
        ESP_ERROR_CHECK(device_write(imu, regs[i], &vals[i], 1));
    if (xTaskCreate(sample_task, "gravity", 3072, NULL, 2, NULL) != pdPASS)
        abort();
    ESP_LOGI("Gravity", "QMI8658 continuous leveling enabled");
}
