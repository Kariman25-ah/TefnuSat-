#include <Arduino.h>
#include "print_lib.h"
#include "RF.h"


void printFullTelemetry() {

    Serial.print(":");
    Serial.print(d1.bmpTemp);    Serial.print(",");
    Serial.print(d1.bmpPress);   Serial.print(",");
    Serial.print(d1.accelX);     Serial.print(",");
    Serial.print(d1.accelY);     Serial.print(",");
    Serial.print(d1.accelZ);     Serial.print(",");
    Serial.print(d1.gyroX);      Serial.print(",");
    Serial.print(d1.gyroY);      Serial.print(",");
    Serial.print(d1.gyroZ);      Serial.print(",");

    Serial.print(":");
    Serial.print(d2.compassAzimuth);    Serial.print(",");
    Serial.print(d2.dhtTemp);           Serial.print(",");
    Serial.print(d2.dhtHum);            Serial.print(",");
    Serial.print(d2.altitude_bmp);      Serial.print(",");
    Serial.print(d2.gpsSatellites_Use); Serial.print(",");
    Serial.print(d2.gpsPosition_Fix);   Serial.print(",");
    Serial.print(d2.gpsLat);            Serial.print(",");
    
    Serial.print(":");
    Serial.print(d3.gpsLon);  Serial.print(",");
    Serial.print(d3.gpsAlt);  Serial.print(",");
    Serial.print(d3.gpsUTC);  Serial.print(",");
    Serial.print(d3.gpsN_S);  Serial.print(",");
    Serial.print(d3.gpsE_W);  Serial.print(",");
    Serial.print(d3.gpsHDOP);

    Serial.println();
}
