#pragma once
#include "board.h"
esp_err_t device_open(uint8_t address, i2c_master_dev_handle_t *dev);
esp_err_t device_read(i2c_master_dev_handle_t dev, uint8_t reg, void *data, size_t length);
esp_err_t device_write(i2c_master_dev_handle_t dev, uint8_t reg, const void *data, size_t length);
void peripherals_init(void);
