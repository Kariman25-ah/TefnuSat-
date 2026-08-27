#ifndef RF_H_
#define RF_H_

#include <Arduino.h>
#include <RF24.h>

// ------𓏏𓍋𓈖𓅱 TefnuSat RF 𓏏𓍋𓈖𓅱------//

#pragma pack(1)
struct Telemetry_1 {
    uint8_t packetID;
    float bmpTemp;
    float bmpPress;
    float accelX;
    float accelY;
    float accelZ;
    int16_t gyroX;
    int16_t gyroY;
    int16_t gyroZ;
};

struct Telemetry_2 {
    uint8_t packetID;
    float compassAzimuth;
    float dhtTemp;
    float dhtHum;
    float altitude_bmp;
    float gpsSatellites_Use;
    float gpsPosition_Fix;
    float gpsLat;
};

struct Telemetry_3 {
    uint8_t packetID;
    float gpsLon;
    float gpsAlt;
    float gpsUTC;
    char  gpsN_S;
    char  gpsE_W;
    float gpsHDOP;
};
#pragma pack()

extern RF24 Tefnu;
extern Telemetry_1 d1;
extern Telemetry_2 d2;
extern Telemetry_3 d3;
extern bool got1, got2, got3;

void init_RF();
bool isRF_OK();
void reinit_RF();

#endif
