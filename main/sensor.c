#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c_master.h"

#define I2C_MASTER_SCL_IO    22
#define I2C_MASTER_SDA_IO    12
#define I2C_MASTER_FREQ_HZ   100000
#define SENSOR_ADDR          0x40  // Likely Si7021 or SHT20/21

static i2c_master_bus_handle_t i2c_bus = NULL;
static i2c_master_dev_handle_t sensor_dev = NULL;

// Initialize new I2C master API
esp_err_t i2c_master_init(void) {
    i2c_master_bus_config_t bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_NUM_0,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &i2c_bus));

    i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = SENSOR_ADDR,
        .scl_speed_hz = I2C_MASTER_FREQ_HZ,
    };

    ESP_ERROR_CHECK(i2c_master_bus_add_device(i2c_bus, &dev_config, &sensor_dev));
    return ESP_OK;
}

// Read 4 bytes from sensor register using new API
esp_err_t read_sensor(uint8_t reg, uint8_t *buf, size_t len) {
    esp_err_t err;

    err = i2c_master_transmit(sensor_dev, &reg, 1, pdMS_TO_TICKS(100));
    if (err != ESP_OK) return err;

    vTaskDelay(pdMS_TO_TICKS(20));  // Give time for measurement

    err = i2c_master_receive(sensor_dev, buf, len, pdMS_TO_TICKS(100));
    return err;
}

esp_err_t read_temperature_and_humidity(float *temperature, float *humidity) {
    uint8_t buf[4];
    esp_err_t err = read_sensor(0x00, buf, 4);
    if (err != ESP_OK) return err;

    uint16_t temp_raw = (buf[0] << 8) | buf[1];
    uint16_t hum_raw  = (buf[2] << 8) | buf[3];

    *temperature = ((float)temp_raw * 165.0 / 65535.0) - 40.0;
    *humidity    = ((float)hum_raw / 65535.0) * 100.0;

    return ESP_OK;
}