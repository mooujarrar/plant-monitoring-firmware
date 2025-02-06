#include "moisture.h"

static const char *TAG = "Moisture sensor";

static void moisture_reading(void *pvParameters) {
    uint16_t adc_data;
    while (1) {
        if (ESP_OK == adc_read(&adc_data)) {
            // Convert the ADC value to a percentage
            int moisture_percentage = fabs(MOISTURE_AIR - adc_data) * 100 / (MOISTURE_AIR - MOISTURE_WATER);
            if (moisture_percentage < 0) {
                moisture_percentage = 0;  // Clamp to 0% if out of range
            } else if (moisture_percentage > 100) {
                moisture_percentage = 100;  // Clamp to 100% if out of range
            }
            ESP_LOGI(TAG, "adc read: %d\r\n", moisture_percentage);
            // TODO: Instead of logging only the data, I should publish them in topic 'moisture' of the mqtt
        }
        // We take a measurement each minute
        vTaskDelay(pdMS_TO_TICKS(6000));
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

    // Instanciate Task to read Moisture sensor values
    xTaskCreate(moisture_reading, "mosture_reading_task", 4096, NULL, 5, NULL);
}
