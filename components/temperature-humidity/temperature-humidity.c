#include "temperature-humidity.h"

static const char *TAG = "Temperature & Humidity sensor";


// Task handle for the sensor reading loop
static TaskHandle_t reading_task_handle = NULL;

// Sensor reading and publishing task
static void sensor_reading_task(void *pvParameters) {
    esp_mqtt_client_handle_t mqtt_client = *(esp_mqtt_client_handle_t *)pvParameters;
    int msg_id;
    char buffer[16];          // Buffer to hold the resulting string
    setDHTgpio(GPIO_NUM_4);
    ESP_LOGI(TAG, "Starting DHT Task\n\n");

    while (1)
    {
        ESP_LOGI(TAG, "=== Reading DHT ===\n");
        int ret = readDHT();

        errorHandler(ret);

        ESP_LOGI(TAG, "Hum: %.1f Tmp: %.1f\n", getHumidity(), getTemperature());

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
     // Initialize NVS
     /*esp_err_t ret = nvs_flash_init();
     if (ret == ESP_ERR_NVS_NO_FREE_PAGES)
     {
         ESP_ERROR_CHECK(nvs_flash_erase());
         ret = nvs_flash_init();
     }
     ESP_ERROR_CHECK(ret);*/
 
     esp_log_level_set("*", ESP_LOG_INFO);
 
     
     // Initiate a configuration for the reading pin
    // Step 1: Configure GPIO4 as an output
    gpio_config_t io_conf = {
        .pin_bit_mask = GPIO_NUM_4,               // Select GPIO4
        .mode = GPIO_MODE_OUTPUT,                 // Set pin as output
        .pull_up_en = GPIO_PULLUP_DISABLE,        // No pull-up resistor
        .pull_down_en = GPIO_PULLDOWN_DISABLE,    // No pull-down resistor
        .intr_type = GPIO_INTR_DISABLE            // No interrupt
    };
    gpio_config(&io_conf);  // Apply configuration

    // Register sensor event handler
    ESP_ERROR_CHECK(esp_event_handler_register(SENSOR_EVENTS, ESP_EVENT_ANY_ID, sensor_event_handler, NULL));
}
