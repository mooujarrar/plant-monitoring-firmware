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
        char humidity_topic[100];
        sprintf(humidity_topic, "/%s/%s", CONFIG_PLANT_TOKEN, CONFIG_PLANT_TOKEN_HUMIDITY);

        char temperature_topic[100];
        sprintf(temperature_topic, "/%s/%s", CONFIG_PLANT_TOKEN, CONFIG_PLANT_TOKEN_TEMPERATURE);

        msg_id = esp_mqtt_client_publish(mqtt_client, humidity_topic, humidity_buffer, 0, 1, 0);
        msg_id = esp_mqtt_client_publish(mqtt_client, temperature_topic, temp_buffer, 0, 1, 0);

        ESP_LOGI(TAG, "Published Humidity=%d.%d, Temperature=%d.%d, msg_id=%d\r\n", humidity.humidity_whole, humidity.humidity_decimal, temperature.temperature_whole, temperature.temperature_decimal, msg_id);

        // -- wait at least 10 sec before reading again ------------
        // The interval of whole process must be beyond 2 seconds !!
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

static void sensor_read_and_publish(esp_mqtt_client_handle_t *mqtt_client) {
    char humidity_buffer[16];  // Buffer to hold the resulting string
    char temp_buffer[16];      // Buffer to hold the resulting string

    // Set the DHT sensor GPIO pin
    setDHTgpio(GPIO_NUM_4);

    // Check that mqtt_client is valid
    if (mqtt_client == NULL) {
        ESP_LOGE(TAG, "MQTT client handle is NULL; cannot publish.");
        return;
    }

    // Read data from the DHT sensor
    int ret = readDHT();

    // Handle any error while reading the DHT sensor
    errorHandler(ret);

    // Get humidity and temperature values
    humidity_t humidity = getHumidity();
    temperature_t temperature = getTemperature();

    // Convert humidity and temperature values to strings
    snprintf(humidity_buffer, sizeof(humidity_buffer), "%d.%d", humidity.humidity_whole, humidity.humidity_decimal);
    snprintf(temp_buffer, sizeof(temp_buffer), "%d.%d", temperature.temperature_whole, temperature.temperature_decimal);

    // Publish the humidity value to the MQTT Broker
    char humidity_topic[100];
    sprintf(humidity_topic, "/%s/%s", CONFIG_PLANT_TOKEN, CONFIG_PLANT_TOKEN_HUMIDITY);
    int msg_id = esp_mqtt_client_publish(*mqtt_client, humidity_topic, humidity_buffer, 0, 1, 0);

    // Publish the temperature value to the MQTT Broker
    char temperature_topic[100];
    sprintf(temperature_topic, "/%s/%s", CONFIG_PLANT_TOKEN, CONFIG_PLANT_TOKEN_TEMPERATURE);
    msg_id = esp_mqtt_client_publish(*mqtt_client, temperature_topic, temp_buffer, 0, 1, 0);

    ESP_LOGI(TAG, "Published Humidity=%d.%d, Temperature=%d.%d, msg_id=%d", 
        humidity.humidity_whole, humidity.humidity_decimal, 
        temperature.temperature_whole, temperature.temperature_decimal, 
        msg_id);
}

// Event loop handler
static void sensor_event_handler(void *handler_arg, esp_event_base_t base, int32_t id, void *event_data) {
    if (base == SENSOR_EVENTS) {
        switch (id) {
            case SENSOR_EVENT_START:
                ESP_LOGI(TAG, "SENSOR_EVENT_START received in DHT22");

                // Simulate data publishing
                ESP_LOGI(TAG, "DHT22 is publishing data...");
                sensor_read_and_publish((esp_mqtt_client_handle_t *)event_data);

                // Post acknowledgment
                uint32_t ack_bit = SENSOR_ACK_BIT_2;
                esp_event_post(SENSOR_EVENTS, SENSOR_EVENT_ACK, &ack_bit, sizeof(ack_bit), portMAX_DELAY);
                break;

            case SENSOR_EVENT_STOP:
                ESP_LOGI(TAG, "SENSOR_EVENT_STOP received");
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
