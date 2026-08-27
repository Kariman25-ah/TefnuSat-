//𓏏𓍋𓈖𓅱 header files
#include <I2Cdev.h>
#include <MPU6050.h>
#include <Adafruit_BMP085.h>
#include <Adafruit_HMC5883_U.h>
#include <Wire.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <RF24.h>
#include <TinyGPSPlus.h>
#include "GPS_lib.h"
#include "dht_lib.h"
#include "RF.h"

// ------𓏏𓍋𓈖𓅱 TefnuSat CubeSat 𓏏𓍋𓈖𓅱------//

//𓏏𓍋𓈖𓅱 GPS pins
SoftwareSerial ss(3, 4);
TinyGPSPlus gps;

//𓏏𓍋𓈖𓅱 init of sensors
MPU6050 accelgyro;
Adafruit_BMP085 bmp;
Adafruit_HMC5883_Unified mag(1);

//𓏏𓍋𓈖𓅱 declaration of variable of motion
int16_t x, y, z;
int16_t gx, gy, gz;

//𓏏𓍋𓈖𓅱 declaration of dht variables 
float dht_t, dht_h;

//𓏏𓍋𓈖𓅱 P0 = standard sea-level pressure in Pa
const float P0 = 101325.0;

//𓏏𓍋𓈖𓅱 Keplerian orbital elements — TefnuSat TEFNU_650
const float OE_a        = 7067850.0;
const float OE_e        = 0.0049;
const float OE_i        = 1.7261;
const float OE_RAAN     = 6.1609;
const float OE_omega    = 5.5820;
const float OE_nu0      = 1.1598;
const float OMEGA_EARTH = 7.2921e-5;

//𓏏𓍋𓈖𓅱 init RF data structure
Telemetry_1 data1;
Telemetry_2 data2;
Telemetry_3 data3;

//𓏏𓍋𓈖𓅱 timing variables
uint8_t sendState = 3;              
unsigned long lastSendAttempt = 0;
unsigned long previousMillis = 0;
unsigned long RFpreviousMillis = 0;
const long interval = 5000;


void setup() {
    Serial.begin(9600);
    Wire.begin();
    ss.begin(9600);

    init_dht(2);

    //𓏏𓍋𓈖𓅱 MPU init
    accelgyro.initialize();
    accelgyro.setI2CBypassEnabled(true);
    Serial.println(accelgyro.testConnection() ?
                   "MPU6050 connection successful" : "MPU6050 connection failed");

    if (!mag.begin())
        Serial.println("HMC5883L not found!");

    //𓏏𓍋𓈖𓅱 BMP init 
    Serial.println("---Initializing BMP---");
    if (!bmp.begin())
        Serial.println("GY-87: BMP085 not found!");

    //𓏏𓍋𓈖𓅱 RF init
    init_RF();
}

void loop() {
    unsigned long currentMillis = millis();

    //𓏏𓍋𓈖𓅱 GPS reading 
    unsigned long gpsStart = millis();
    while (millis() - gpsStart < 20) {
        while (ss.available() > 0)
            gps.encode(ss.read());
    }

    //𓏏𓍋𓈖𓅱 RF watchdog
    if (!isRF_OK()) {
        if (currentMillis - RFpreviousMillis >= 5000) {
            RFpreviousMillis = currentMillis;
            reinit_RF();
        }
    }

    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;

        // ---𓏏𓍋𓈖𓅱 1st Packet ---
        data1.packetID = 1
        data1.bmpTemp  = bmp.readTemperature();
        data1.bmpPress = bmp.readPressure();
        accelgyro.getMotion6(&x, &y, &z, &gx, &gy, &gz);
        data1.accelX = x / 16384.0;
        data1.accelY = y / 16384.0;
        data1.accelZ = z / 16384.0;
        data1.gyroX  = gx;
        data1.gyroY  = gy;
        data1.gyroZ  = gz;

        // ---𓏏𓍋𓈖𓅱 2nd Packet ---
        data2.packetID = 2;
        sensors_event_t event;
        mag.getEvent(&event);
        float heading = atan2(event.magnetic.y, event.magnetic.x);
        if (heading < 0) heading += 2 * PI;
        data2.compassAzimuth = heading * 180.0 / PI;

        if (read_dht(2, &dht_t, &dht_h) == 0) {
            data2.dhtTemp = dht_t;
            data2.dhtHum  = dht_h;
        }
        data2.altitude_bmp     = 44330.0 * (1.0 - pow(bmp.readPressure() / 101325.0, 1.0 / 5.255));
        data2.gpsSatellites_Use = gps.satellites.isValid() ? (float)gps.satellites.value() : 0.0;

        // ---𓏏𓍋𓈖𓅱 3rd Packet ---
        data3.packetID = 3;

        if (gps.location.isValid()) {
            data2.gpsPosition_Fix = 1.0;
            data2.gpsLat          = (float)gps.location.lat();
            data3.gpsLon          = (float)gps.location.lng();
            data3.gpsAlt          = gps.altitude.isValid() ? (float)gps.altitude.meters() : 0.0;
            data3.gpsN_S          = gps.location.rawLat().negative ? 'S' : 'N';
            data3.gpsE_W          = gps.location.rawLng().negative ? 'W' : 'E';

        } else {
            // 𓏏𓍋𓈖𓅱 Orbital propagation fallback
            data2.gpsPosition_Fix = 2.0;

            float orb_t  = millis() / 1000.0;
            float nu     = OE_nu0 + 0.001059 * orb_t;
            float r      = OE_a * (1.0 - OE_e * OE_e) / (1.0 + OE_e * cos(nu));

            float x_orb  = r * cos(nu);
            float y_orb  = r * sin(nu);

            float X = x_orb * (cos(OE_RAAN)*cos(OE_omega) - sin(OE_RAAN)*sin(OE_omega)*cos(OE_i))
                    - y_orb * (cos(OE_RAAN)*sin(OE_omega) + sin(OE_RAAN)*cos(OE_omega)*cos(OE_i));
            float Y = x_orb * (sin(OE_RAAN)*cos(OE_omega) + cos(OE_RAAN)*sin(OE_omega)*cos(OE_i))
                    - y_orb * (sin(OE_RAAN)*sin(OE_omega) - cos(OE_RAAN)*cos(OE_omega)*cos(OE_i));
            float Z = x_orb * (sin(OE_omega)*sin(OE_i))
                    + y_orb * (cos(OE_omega)*sin(OE_i));

            float theta  = OMEGA_EARTH * orb_t;
            float X_ecef =  X * cos(theta) + Y * sin(theta);
            float Y_ecef = -X * sin(theta) + Y * cos(theta);
            float Z_ecef =  Z;

            float p      = sqrt(X_ecef*X_ecef + Y_ecef*Y_ecef);
            data2.gpsLat = atan2(Z_ecef, p) * 180.0 / PI;
            data3.gpsLon = atan2(Y_ecef, X_ecef) * 180.0 / PI;
            data3.gpsAlt = (sqrt(X_ecef*X_ecef + Y_ecef*Y_ecef + Z_ecef*Z_ecef) - 6371000.0) / 1000.0;
            data3.gpsN_S = (data2.gpsLat >= 0) ? 'N' : 'S';
            data3.gpsE_W = (data3.gpsLon >= 0) ? 'E' : 'W';
        }

        data3.gpsUTC  = gps.time.isValid() ?
                        (float)(gps.time.hour() * 10000 + gps.time.minute() * 100 + gps.time.second()) : 0.0;
        data3.gpsHDOP = gps.hdop.isValid() ? (float)gps.hdop.hdop() : 0.0;

        sendState = 0;

        // 𓏏𓍋𓈖𓅱 Serial debug
        Serial.print("1st packet: ");
        Serial.print(data1.bmpTemp);   Serial.print(",");
        Serial.print(data1.bmpPress);  Serial.print(",");
        Serial.print(data1.accelX);    Serial.print(",");
        Serial.print(data1.accelY);    Serial.print(",");
        Serial.print(data1.accelZ);    Serial.print(",");
        Serial.print(data1.gyroX);     Serial.print(",");
        Serial.print(data1.gyroY);     Serial.print(",");
        Serial.println(data1.gyroZ);

        Serial.print("2nd packet: ");
        Serial.print(data2.compassAzimuth);    Serial.print(",");
        Serial.print(data2.dhtTemp);           Serial.print(",");
        Serial.print(data2.dhtHum);            Serial.print(",");
        Serial.print(data2.altitude_bmp);      Serial.print(",");
        Serial.print(data2.gpsSatellites_Use); Serial.print(",");
        Serial.print(data2.gpsPosition_Fix);   Serial.print(",");
        Serial.print(data2.gpsLat);            Serial.print(",");


        Serial.print("3rd packet: ");
            Serial.print(data3.gpsLon);  Serial.print(",");
         Serial.print(data3.gpsAlt);  Serial.print(",");
         Serial.print(data3.gpsUTC);  Serial.print(",");
         Serial.print(data3.gpsN_S);  Serial.print(",");
         Serial.print(data3.gpsE_W);  Serial.print(",");
         Serial.print(data3.gpsHDOP);
    }

    if (sendState < 3) {
        send_RF(data1, data2, data3, sendState, lastSendAttempt);
    }
}
