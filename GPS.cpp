#include <Arduino.h>
#include <SoftwareSerial.h>
#include <TinyGPSPlus.h>
#include "GPS_lib.h"
 
void read_gps() {
    while (ss.available() > 0) {
        char c = ss.read();
        gps.encode(c);
    }
}
