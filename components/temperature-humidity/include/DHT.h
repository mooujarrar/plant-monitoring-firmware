/* 
	DHT22 temperature sensor driver
*/

#ifndef DHT_H_
#define DHT_H_

#define DHT_OK 0
#define DHT_CHECKSUM_ERROR -1
#define DHT_TIMEOUT_ERROR -2

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// == function prototypes =======================================

typedef struct {
    int humidity_whole;
    int humidity_decimal;
} humidity_t;

typedef struct {
    int temperature_whole;
    int temperature_decimal;
} temperature_t;

void setDHTgpio(int gpio);
void errorHandler(int response);
int readDHT();
humidity_t getHumidity();
temperature_t getTemperature();
int getSignalLevel(int usTimeOut, bool state);

#endif