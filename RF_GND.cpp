#include <SPI.h>
#include <RF24.h>
#include "RF.h"

Telemetry_1 d1;
Telemetry_2 d2;
Telemetry_3 d3;

bool got1 = false;
bool got2 = false;
bool got3 = false;


RF24 Tefnu(9, 10);
const byte addresses[][6] = {"ADR12", "ADR13"};

static void applyConfig() {
    Tefnu.enableDynamicPayloads();
    Tefnu.setAutoAck(false);
    Tefnu.openWritingPipe(addresses[0]);
    Tefnu.openReadingPipe(1, addresses[0]);
    Tefnu.setPALevel(RF24_PA_MIN);
    Tefnu.setDataRate(RF24_250KBPS);
    Tefnu.stopListening();
}

void init_RF() {
    if (!Tefnu.begin()) { Serial.println("RF not found!"); return; }
    applyConfig();
}

bool isRF_OK() { return Tefnu.isChipConnected(); }

void reinit_RF() {
    if (Tefnu.begin()) { applyConfig(); Serial.println("RF Re-init OK."); }
    else Serial.println("RF Re-init failed.");
}
