#ifndef HUM_TEMP_H
#define HUM_TEMP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"
#include "esp_log.h"
#include "esp_event.h"
#include "mqtt_client.h"
#include "events.h"
#include "esp_system.h"
#include "freertos/event_groups.h"
#include "driver/gpio.h"
#include "nvs_flash.h"

#include "DHT.h"

// Define API
void reading_init(void);

#endif // HUM_TEMP_H