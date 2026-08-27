#include <Arduino.h>
#include <SPI.h>
#include <RF24.h>
#include "RF.h"

// ------ 𓏏𓍋𓈖𓅱 TefnuSat RF 𓏏𓍋𓈖𓅱 ------//

#define CE_PIN  9
#define CSN_PIN 10

// 𓏏𓍋𓈖𓅱 listen timeout in milliseconds
#define LISTEN_TIMEOUT_MS 200UL

RF24 Tefnu(CE_PIN, CSN_PIN);

const byte addresses[][6] = {"ADR12", "ADR13"};

char command = 0;

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
    if (!Tefnu.begin()) {
        Serial.println("RF Hardware not responding!");
        return;
    }
    applyConfig();
    
    Serial.println("RF Ready.");
}

bool isRF_OK() {
    return Tefnu.isChipConnected();
}

void reinit_RF() {
    if (Tefnu.begin()) {
        Tefnu.enableDynamicPayloads();
        applyConfig();
        Serial.println("RF Re-initialized.");
    } else {
        Serial.println("RF Re-init failed.");
    }
}

void send_RF(Telemetry_1 &data1, Telemetry_2 &data2, Telemetry_3 &data3,
             uint8_t &sendState, unsigned long &lastSendAttempt) {

    unsigned long now = millis();
    Tefnu.stopListening();

    if (sendState == 0) {
        Tefnu.write(&data1, sizeof(data1));
        lastSendAttempt = now;
        sendState = 1;
    }
    else if (sendState == 1 && now - lastSendAttempt >= 50) {
        Tefnu.write(&data2, sizeof(data2));
        lastSendAttempt = now;
        sendState = 2;
    }
    else if (sendState == 2 && now - lastSendAttempt >= 50) {
        Tefnu.write(&data3, sizeof(data3));
        lastSendAttempt = now;
        sendState = 3;
    }
}

char receive_RF() {
    Tefnu.startListening();

    unsigned long start = millis();
    while (!Tefnu.available()) {
        if (millis() - start >= LISTEN_TIMEOUT_MS) {
            // 𓏏𓍋𓈖𓅱 nothing arrived; return to TX mode
            Tefnu.stopListening();
            return 0;
        }
    }

    Tefnu.read(&command, sizeof(command));
    Tefnu.stopListening();
    Serial.print("Command Received: ");
    Serial.println(command);
    return command;
}
