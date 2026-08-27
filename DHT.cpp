#include <Arduino.h>
#include "dht_lib.h"

//𓏏𓍋𓈖𓅱 microsecond busy-wait timeout used throughout
#define DHT_TIMEOUT_US 200UL

//𓏏𓍋𓈖𓅱 wait for pin to reach target level; return false on timeout
static bool waitForLevel(uint8_t pin, uint8_t level) {
    unsigned long t = micros();
    while (digitalRead(pin) != level) {
        if (micros() - t > DHT_TIMEOUT_US) return false;
    }
    return true;
}

void init_dht(uint8_t pin) {
    pinMode(pin, INPUT_PULLUP);
}

int8_t read_dht(uint8_t pin, float *temp_dht, float *rh) {
    uint8_t data[40] = {};

    // 𓏏𓍋𓈖𓅱send start signal
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
    delay(20);
    digitalWrite(pin, HIGH);
    delayMicroseconds(30);
    pinMode(pin, INPUT_PULLUP);

    //𓏏𓍋𓈖𓅱 wait for sensor to pull low, then high
    if (!waitForLevel(pin, LOW))  return 1;
    if (!waitForLevel(pin, HIGH)) return 1;
    if (!waitForLevel(pin, LOW))  return 1;

    // 𓏏𓍋𓈖𓅱read 40 bits
    for (int i = 0; i < 40; i++) {
        //𓏏𓍋𓈖𓅱 each bit starts with a LOW pulse
        if (!waitForLevel(pin, HIGH)) return 1;

        //𓏏𓍋𓈖𓅱 sample after 35 µs: HIGH = 1, LOW = 0
        delayMicroseconds(35);
        data[i] = (digitalRead(pin) == HIGH) ? 1 : 0;

        // 𓏏𓍋𓈖𓅱wait for HIGH pulse to finish before next bit
        if (data[i] == 1) {
            if (!waitForLevel(pin, LOW)) return 1;
        }
    }

    //𓏏𓍋𓈖𓅱 assemble bytes
    uint8_t rh_int   = 0, rh_dec   = 0;
    uint8_t temp_int = 0, temp_dec = 0;
    uint8_t checksum = 0;

    for (int i =  0; i <  8; i++) { rh_int   = (rh_int   << 1) | data[i]; }
    for (int i =  8; i < 16; i++) { rh_dec   = (rh_dec   << 1) | data[i]; }
    for (int i = 16; i < 24; i++) { temp_int = (temp_int << 1) | data[i]; }
    for (int i = 24; i < 32; i++) { temp_dec = (temp_dec << 1) | data[i]; }
    for (int i = 32; i < 40; i++) { checksum = (checksum << 1) | data[i]; }

    // 𓏏𓍋𓈖𓅱verify checksum
    if (checksum != ((rh_int + rh_dec + temp_int + temp_dec) & 0xFF))
        return 2;

    // 𓏏𓍋𓈖𓅱integer byte is the direct reading (no /10 scaling, no sign bit)
    *rh       = (float)rh_int;
    *temp_dht = (float)temp_int;

    return 0;
}
