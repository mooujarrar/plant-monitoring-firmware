#ifndef EVENTS_H
#define EVENTS_H

#include "esp_event.h"

// Declare the event base
ESP_EVENT_DECLARE_BASE(SENSOR_EVENTS);

// Declare event types
typedef enum {
    SENSOR_EVENT_START,  // Triggered to start moisture reading task
    SENSOR_EVENT_STOP,   // Triggered to stop moisture reading task
} reading_rvent_t;

#endif // EVENTS_H