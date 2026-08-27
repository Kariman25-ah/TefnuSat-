#include <SPI.h>
#include <RF24.h>
#include "RF.h"
#include "print_lib.h"



void setup() {
    Serial.begin(9600);
    init_RF();
    Tefnu.startListening();
}

void loop() {

    if (Tefnu.available()) {
        uint8_t len = Tefnu.getDynamicPayloadSize();
        
        if (len > 0) {
            uint8_t buffer[32];
            Tefnu.read(buffer, len);
            uint8_t packetID = buffer[0];

            if (packetID == 1 && len == sizeof(Telemetry_1)) {
                memcpy(&d1, buffer, sizeof(d1));
                got1 = true;
            }
            else if (packetID == 2 && len == sizeof(Telemetry_2)) {
                memcpy(&d2, buffer, sizeof(d2));
                got2 = true;
            }
            else if (packetID == 3 && len == sizeof(Telemetry_3)) {
                memcpy(&d3, buffer, sizeof(d3));
                got3 = true;
            }

        
            if (got1 && got2 && got3) {
                printFullTelemetry();
                got1 = false;
                got2 = false;
                got3 = false;
            }
        }
    }
}
