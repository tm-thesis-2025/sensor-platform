#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_log.h"

#define SOIL_ADC_CHANNEL     ADC1_CHANNEL_0   // GPIO1 on ESP32-H2
#define SOIL_POWER_GPIO      GPIO_NUM_12       // GPIO used to power the soil sensor

static const char *TAG = "SoilSensor";

// Initialize ADC and GPIO
void soil_sensor_init() {
    gpio_reset_pin(SOIL_POWER_GPIO);
    gpio_set_direction(SOIL_POWER_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(SOIL_POWER_GPIO, 0); // Turn off initially
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(SOIL_ADC_CHANNEL, ADC_ATTEN_DB_11);
}

// Read soil moisture level
int read_soil_moisture() {
    gpio_set_level(SOIL_POWER_GPIO, 1);
    vTaskDelay(pdMS_TO_TICKS(10));

    int adc_reading = adc1_get_raw(SOIL_ADC_CHANNEL);

    gpio_set_level(SOIL_POWER_GPIO, 0);
    return adc_reading;
}