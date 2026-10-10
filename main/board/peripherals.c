#include "peripherals.h"
#include "esp_log.h"
#include <string.h>
esp_err_t device_open(uint8_t address, i2c_master_dev_handle_t *dev) {
    i2c_device_config_t cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7, .device_address = address, .scl_speed_hz = 400000};
    return i2c_master_bus_add_device(board_i2c_handle(), &cfg, dev);
}
esp_err_t device_read(i2c_master_dev_handle_t dev, uint8_t reg, void *data, size_t length) {
    return i2c_master_transmit_receive(dev, &reg, 1, data, length, 100);
}
esp_err_t device_write(i2c_master_dev_handle_t dev, uint8_t reg, const void *data, size_t length) {
    uint8_t bytes[17];
    if (length > 16)
        return ESP_ERR_INVALID_SIZE;
    bytes[0] = reg;
    memcpy(bytes + 1, data, length);
    return i2c_master_transmit(dev, bytes, length + 1, 100);
}
void peripherals_init(void) {
    i2c_master_dev_handle_t pmic, imu;
    uint8_t id = 0, value = 0;
    ESP_ERROR_CHECK(device_open(0x34, &pmic));
    if (device_read(pmic, 0x03, &id, 1) == ESP_OK && (id & 0xcf) == 0x4a) {
        ESP_LOGI("PMIC", "AXP2101 detected; preserve factory power rails/charging limits");
        /* Official XPowersLib: ADC_CHANNEL_CTRL 0x30 bit 1 = TS measurement.
           This board has no battery NTC; do not change supply voltages. */
        if (device_read(pmic, 0x30, &value, 1) == ESP_OK) {
            value &= ~2;
            ESP_ERROR_CHECK(device_write(pmic, 0x30, &value, 1));
        }
    } else
        ESP_LOGW("PMIC", "AXP2101 not detected (id=%02x)", id);
    ESP_ERROR_CHECK(device_open(0x6b, &imu));
    if (device_read(imu, 0, &id, 1) == ESP_OK && id == 5) {
        value = 0;
        ESP_ERROR_CHECK(device_write(imu, 0x08, &value, 1));
        ESP_LOGI("IMU", "QMI8658 verified; gravity task will enable accelerometer");
    } else
        ESP_LOGW("IMU", "QMI8658 not detected");
    ESP_ERROR_CHECK(i2c_master_bus_rm_device(imu));
    ESP_ERROR_CHECK(i2c_master_bus_rm_device(pmic));
}
