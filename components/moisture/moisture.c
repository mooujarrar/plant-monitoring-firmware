#include "moisture.h"

static const char *TAG = "Moisture sensor";

// Define MOISTURE_EVENTS here
ESP_EVENT_DEFINE_BASE(MOISTURE_EVENTS);

// Task handle for the moisture reading loop
static TaskHandle_t moisture_task_handle = NULL;

// Moisture reading and publishing task
static void moisture_reading_task(void *pvParameters) {
    esp_mqtt_client_handle_t mqtt_client = *(esp_mqtt_client_handle_t *)pvParameters;
    int msg_id;
    uint16_t adc_data;
    char buffer[16];          // Buffer to hold the resulting string
    while (1) {
        // Check that mqtt_client is valid
        if (mqtt_client == NULL) {
            ESP_LOGE(TAG, "MQTT client handle is NULL; cannot publish.");
            vTaskDelay(pdMS_TO_TICKS(1000)); // Delay before retry
            continue;
        }
        if (ESP_OK == adc_read(&adc_data)) {
            // Convert the ADC value to a percentage
            int moisture_percentage = fabs(MOISTURE_AIR - adc_data) * 100 / (MOISTURE_AIR - MOISTURE_WATER);
            if (moisture_percentage < 0) {
                moisture_percentage = 0;  // Clamp to 0% if out of range
            } else if (moisture_percentage > 100) {
                moisture_percentage = 100;  // Clamp to 100% if out of range
            }
            // Put the read value into a char[] buffer
            itoa(moisture_percentage, buffer, 10);
            // Publish the moisture value to MQTT Broker
            msg_id = esp_mqtt_client_publish(mqtt_client, "/moisture", buffer, 0, 1, 0);
            ESP_LOGI(TAG, "Published moisture=%d, msg_id=%d\r\n", moisture_percentage, msg_id);
        }
        // We take a measurement each <To Be decided>
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

// Event loop handler
static void moisture_event_handler(void *handler_arg, esp_event_base_t base, int32_t id, void *event_data) {
    if (base == MOISTURE_EVENTS) {
        switch (id) {
            case MOISTURE_SENSOR_EVENT_START:
                ESP_LOGI(TAG, "MOISTURE_SENSOR_EVENT_START received");

                // Start or resume the moisture reading task
                if (moisture_task_handle == NULL) {
                    xTaskCreate(moisture_reading_task, "moisture_task", 4096, event_data, 5, &moisture_task_handle);
                } else {
                    vTaskResume(moisture_task_handle);
                }
                break;

            case MOISTURE_SENSOR_EVENT_STOP:
                ESP_LOGI(TAG, "MOISTURE_SENSOR_EVENT_STOP received");

                // Suspend the moisture reading task
                if (moisture_task_handle != NULL) {
                    vTaskSuspend(moisture_task_handle);
                }
                break;

            default:
                break;
        }
    }
}

void moisture_init(void) {
    // 1. init adc
    adc_config_t adc_config;

    // Depend on menuconfig->Component config->PHY->vdd33_const value
    // When measuring system voltage(ADC_READ_VDD_MODE), vdd33_const must be set to 255.
    adc_config.mode = ADC_READ_TOUT_MODE;
    adc_config.clk_div = 16; // ADC sample collection clock = 160MHz/clk_div = 10MHz
    ESP_ERROR_CHECK(adc_init(&adc_config));

    // Register moisture event handler
    ESP_ERROR_CHECK(esp_event_handler_register(MOISTURE_EVENTS, ESP_EVENT_ANY_ID, moisture_event_handler, NULL));
}
