#include "temperature-humidity.h"

static const char *TAG = "Temperature & Humidity sensor";


// Task handle for the sensor reading loop
static TaskHandle_t reading_task_handle = NULL;

// Sensor reading and publishing task
static void sensor_reading_task(void *pvParameters) {
    esp_mqtt_client_handle_t mqtt_client = *(esp_mqtt_client_handle_t *)pvParameters;
    int msg_id;
    char humidity_buffer[16];          // Buffer to hold the resulting string
    char temp_buffer[16];          // Buffer to hold the resulting string
    setDHTgpio(GPIO_NUM_4);
    ESP_LOGI(TAG, "Starting DHT Task\n\n");
    while (1)
    {
        // Check that mqtt_client is valid
        if (mqtt_client == NULL) {
            ESP_LOGE(TAG, "MQTT client handle is NULL; cannot publish.");
            vTaskDelay(pdMS_TO_TICKS(1000)); // Delay before retry
            continue;
        }
        int ret = readDHT();

        errorHandler(ret);

        humidity_t humidity = getHumidity();
        temperature_t temperature = getTemperature();

        // Put the read value into a char[] buffer
        snprintf(humidity_buffer, sizeof(humidity_buffer), "%d.%d", humidity.humidity_whole, humidity.humidity_decimal);
        snprintf(temp_buffer, sizeof(temp_buffer), "%d.%d", temperature.temperature_whole, temperature.temperature_decimal);

        // Publish the moisture value to MQTT Broker
        msg_id = esp_mqtt_client_publish(mqtt_client, "/humidity", humidity_buffer, 0, 1, 0);
        msg_id = esp_mqtt_client_publish(mqtt_client, "/temperature", temp_buffer, 0, 1, 0);

        ESP_LOGI(TAG, "Published Humidity=%d.%d, Temperature=%d.%d, msg_id=%d\r\n", humidity.humidity_whole, humidity.humidity_decimal, temperature.temperature_whole, temperature.temperature_decimal, msg_id);

        // -- wait at least 10 sec before reading again ------------
        // The interval of whole process must be beyond 2 seconds !!
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

// Event loop handler
static void sensor_event_handler(void *handler_arg, esp_event_base_t base, int32_t id, void *event_data) {
    if (base == SENSOR_EVENTS) {
        switch (id) {
            case SENSOR_EVENT_START:
                ESP_LOGI(TAG, "SENSOR_EVENT_START received");

                // Start or resume the sensor reading task
                if (reading_task_handle == NULL) {
                    xTaskCreate(sensor_reading_task, "sensor_reading_task", 4096, event_data, 5, &reading_task_handle);
                } else {
                    vTaskResume(reading_task_handle);
                }
                break;

            case SENSOR_EVENT_STOP:
                ESP_LOGI(TAG, "SENSOR_EVENT_STOP received");

                // Suspend the reading task
                if (reading_task_handle != NULL) {
                    vTaskSuspend(reading_task_handle);
                }
                break;

            default:
                break;
        }
    }
}

void reading_init(void) {     
    // Initiate a configuration for the reading pin
    gpio_pad_select_gpio(GPIO_NUM_4);

    // Register sensor event handler
    ESP_ERROR_CHECK(esp_event_handler_register(SENSOR_EVENTS, ESP_EVENT_ANY_ID, sensor_event_handler, NULL));
}
