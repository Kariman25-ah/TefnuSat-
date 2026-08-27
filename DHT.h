#ifndef DHT_LIB_H
#define DHT_LIB_H

#include <Arduino.h>


void    init_dht(uint8_t pin);
int8_t  read_dht(uint8_t pin, float *temp_dht, float *rh);

#endif
