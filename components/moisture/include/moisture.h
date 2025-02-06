#ifndef MOISTURE_H
#define MOISTURE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"
#include "esp_log.h"
#include "esp_event.h"
#include "mqtt_client.h"
#include <math.h>

// Calibration values (update if sensor behavior changes)
#define MOISTURE_AIR   852  // ADC value for dry air
#define MOISTURE_WATER 432  // ADC value for fully submerged

// Declare MOISTURE_EVENTS as external
ESP_EVENT_DECLARE_BASE(MOISTURE_EVENTS);

typedef enum {
    MOISTURE_SENSOR_EVENT_START,  // Triggered to start moisture reading task
    MOISTURE_SENSOR_EVENT_STOP,   // Triggered to stop moisture reading task
} moisture_event_t;

// Define API
void moisture_init(void);

#endif // MOISTURE_H