#include <stdio.h>
#include <string.h>
#include <time.h>
#include "esp_log.h"
#include "esp_http_client.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sensor2.c"

#define API_URL "http://{REPLACE_WITH_YOUR_OWN_WORKER_LINK}.workers.dev/api/v1/sensor/data"
#define AUTH_TOKEN "Bearer secretkey2"
#define SENSOR_ID "200"
#define INTERVAL_SECONDS 15

static void send_sensor_data_task(void *pvParameters) {

    while (1) {
        int soil_moisture = read_soil_moisture();

        char post_data[256];
        snprintf(post_data, sizeof(post_data),
                 "{\"sensor_id\":\"%s\",\"values\":{\"soil_moisture\":%d}}",
                 SENSOR_ID, soil_moisture);

		esp_http_client_config_t config = {
			.url = API_URL,
			.transport_type = HTTP_TRANSPORT_OVER_SSL,
			.cert_pem = NULL,
			.skip_cert_common_name_check = true,
			.use_global_ca_store = false,       // Make sure this is NOT set
			.disable_auto_redirect = false,
			.keep_alive_enable = true,
			.timeout_ms = 5000,
			.event_handler = NULL,              // Optional: can omit or customize
		};

        esp_http_client_handle_t client = esp_http_client_init(&config);
        esp_http_client_set_method(client, HTTP_METHOD_POST);
        esp_http_client_set_header(client, "Content-Type", "application/json");
        esp_http_client_set_header(client, "Authorization", AUTH_TOKEN);
        esp_http_client_set_post_field(client, post_data, strlen(post_data));

        ESP_LOGI(TAG, "Sending JSON: %s", post_data);

        esp_err_t err = esp_http_client_perform(client);
        if (err == ESP_OK) {
            int status = esp_http_client_get_status_code(client);
            ESP_LOGI(TAG, "POST succeeded, status = %d", status);
        } else {
            ESP_LOGE(TAG, "POST failed: %s", esp_err_to_name(err));
        }

        esp_http_client_cleanup(client);
        vTaskDelay(pdMS_TO_TICKS(INTERVAL_SECONDS * 1000));
    }
}

void start_sensor_task() {
    xTaskCreate(&send_sensor_data_task, "send_sensor_data_task", 8192, NULL, 5, NULL);
}