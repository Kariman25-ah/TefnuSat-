#ifndef GPS_LIB_H
#define GPS_LIB_H
 
#include <Arduino.h>
#include <SoftwareSerial.h>
#include <TinyGPSPlus.h>
 
extern SoftwareSerial ss;
extern TinyGPSPlus gps;
 
void read_gps();
 
#endif
