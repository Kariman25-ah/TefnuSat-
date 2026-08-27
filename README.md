# TefnuSat-
A CubeSat dedicated to monitoring and observing coastal erosion along Egypt's northern coast.
# TefnuSat — Embedded Systems Track

Firmware for TefnuSat's on-board computer (OBC) and sensor suite: an Arduino Nano–based flight computer paired with an ESP32-CAM imaging payload, streaming real-time telemetry to a ground station over nRF24L01.

TefnuSat is a CubeSat mission built to monitor coastal erosion along Egypt's northern coastline. This track was responsible for sensor selection and integration, and for the embedded software that handles onboard data processing, telemetry, and autonomous mission control.

**Prepared by:** Alaa Ramadan, Kariman Ahmed

---

## System Overview

| Component | Role |
|---|---|
| **Arduino Nano (OBC)** | Reads all onboard sensors, packs telemetry into structs, and transmits it over RF every 5 seconds |
| **ESP32-CAM (payload)** | Captures coastline images, stores them to a microSD card, and uploads them to a cloud dashboard |
| **Arduino Nano (Ground Station)** | Receives and validates telemetry packets, logs the full data set over Serial |

## Sensors

| Sensor | Purpose |
|---|---|
| **NEO-6M GPS** | Latitude/longitude, altitude, UTC time, fix quality, HDOP |
| **GY-87** (BMP180 + MPU6050 + magnetometer) | Barometric altitude, 3-axis acceleration, 3-axis gyroscope, compass heading |
| **DHT11** | Ambient temperature and humidity |
| **ESP32-CAM (OV2640)** | Coastline imaging — primary mission payload |
| **nRF24L01** | RF link between TefnuSat and the ground station |

## Firmware Highlights

- **BMP180:** pressure converted to altitude for a sea-level comparison more precise than raw GPS altitude; also reports die temperature for component health monitoring.
- **MPU6050 accelerometer:** raw ±2g 16-bit output converted to g-units on board (`raw / 16384`) for readable telemetry.
- **MPU6050 gyroscope:** sent down as raw 16-bit values; conversion is left to the ground station.
- **Magnetometer:** interfaced via the QMC5883LCompass library; azimuth is computed on board and transmitted as a raw, unrounded float.
- **DHT11:** driven with a custom single-wire bit-banged driver (no library) that validates each reading against its checksum byte.
- **GPS:** read via `SoftwareSerial` + `TinyGPSPlus`, with fix-quality and HDOP checks before any coordinate is trusted.
- **Orbital propagation fallback:** if the GPS loses its fix, the firmware propagates TefnuSat's position from six hard-coded Keplerian elements (same method used by NASA/NORAD-style orbit models) and flags the packet (`gpsPosition_Fix = 2.0`) so the ground station knows the coordinates are computed, not measured.
- **Camera payload:** ESP32-CAM captures SVGA images (CIF fallback without PSRAM), timestamps them via NTP-synced RTC, saves to microSD (`SD_MMC`, 4-bit mode with 1-bit fallback), and uploads them over HTTPS to a Vercel-hosted dashboard in 1 KB chunks.

## Telemetry Protocol

The nRF24L01 can only send 32 bytes per transmission, but a full telemetry frame is larger, so the data is split into three packed structs:

| Struct | Size | `#pragma pack(1)` |
|---|---|---|
| `Telemetry_1` | 27 bytes | ✅ |
| `Telemetry_2` | 29 bytes | ✅ |
| `Telemetry_3` | 19 bytes | ✅ |

- Each struct starts with a 1-byte `packetID` (1/2/3) identifying it to the receiver.
- Field types: `float` (4B) for decimal sensor values, `int16_t` (2B) for raw gyro axes, `char` (1B) for the N/S/E/W hemisphere indicators.
- `#pragma pack(1)` removes compiler padding so sender and receiver interpret the bytes identically.
- Compass azimuth is carried in `Telemetry_2`; the eight GPS fields (lat, lon, GPS altitude, UTC, satellites, fix quality, HDOP, hemisphere chars) are split across `Telemetry_2` and `Telemetry_3`.
- See `RF.h` for the exact field-by-field layout of each struct.

**RF link configuration**

| Parameter | Value |
|---|---|
| Connection | SPI — CE → D9, CSN → D10 |
| Data rate | 250 kbps (max range) |
| Payload mode | Dynamic (`enableDynamicPayloads()`) |
| Send cycle | All 3 packets refreshed every 5 s, sent one at a time via `send_RF()` in `RF.cpp`, staggered 50 ms apart |
| Receiver validation | Checks `packetID` and verifies length with `sizeof()`; corrupt/truncated packets are discarded |
| Receiver output | Once all 3 packets (`got1`, `got2`, `got3`) arrive, `printFullTelemetry()` logs one comma-separated line over Serial |

## Suggested Repository Structure

> Inferred from the report — rename to match your actual files before/after pushing.

```
embedded/
├── OBC_main/
│   └── OBC_main.ino        # Main flight firmware loop (Arduino Nano)
├── RF.h                    # Telemetry_1/2/3 struct definitions
├── RF.cpp                  # init_RF() / send_RF()
├── sensors/
│   ├── BMP180.cpp / .h
│   ├── MPU6050.cpp / .h
│   ├── Magnetometer.cpp / .h
│   ├── GPS.cpp / .h
│   └── DHT11.cpp / .h
├── ESP32_CAM/
│   └── camera_payload.ino  # Imaging payload firmware
└── ground_station/
    └── GS_receiver.ino     # RF receiver + telemetry logger
```

## Libraries Used

- `TinyGPSPlus` — NMEA sentence parsing
- `SoftwareSerial` — dedicated GPS UART channel
- `QMC5883LCompass` — magnetometer I²C interface
- `RF24` — nRF24L01 driver
- `SD_MMC` — ESP32 microSD access
- `WiFiClientSecure` — ESP32-CAM cloud upload
- `esp_camera` — OV2640 camera driver
- `Wire` — I²C bus (GY-87)

## Wiring Summary

| Connection | Interface |
|---|---|
| nRF24L01 ↔ Arduino Nano | SPI, CE → D9, CSN → D10 |
| GY-87 ↔ Arduino Nano | I²C (SDA/SCL) |
| NEO-6M GPS ↔ Arduino Nano | SoftwareSerial, 9600 bps (hardware serial kept free for USB debug) |
| DHT11 ↔ Arduino Nano | Single-wire custom protocol |
| ESP32-CAM | OV2640 pins per `esp_camera_init()` config; images stored via onboard microSD slot |

## Getting Started

1. Install the **Arduino IDE**, the **ESP32 board package** (for the camera payload), and the standard **AVR board package** (for the Nano OBC and ground station).
2. Install the libraries listed above via the Library Manager.
3. Open `OBC_main.ino`, select **Arduino Nano**, and upload.
4. Open the ESP32-CAM sketch, select the matching **ESP32-CAM** board variant, and upload — see the *Security Notes* below before setting Wi-Fi credentials.
5. Open `GS_receiver.ino` on a second Arduino wired identically to the nRF24L01 (CE → D9, CSN → D10), upload, and open the Serial Monitor to view incoming telemetry.

## Security Notes

- Wi-Fi credentials, the cloud endpoint URL, and any API keys currently live as plain constants in the firmware. Move them into a local, **git-ignored** config/secrets header before pushing to a public repository.
- The ESP32-CAM currently disables TLS certificate verification (`client.setInsecure()`). That's acceptable for a prototype talking to a known endpoint, but should be revisited before any production/public deployment.
- Add a `.gitignore` that excludes build artifacts (`*.o`, `*.hex`, `*.elf`, `Debug/`, `Release/`) and any secrets file.

## Team — Embedded Systems Track

- Alaa Ramadan
- Kariman Ahmed

## License

No license specified yet 
