# ESP32-GPS-Bluetooth

ESP32 GPS telemetry parser using UART + Bluetooth Classic.

Reads raw NMEA data from a GPS module, parses location/satellite information, and streams it to:

- USB Serial
- Bluetooth Serial

Supports:

- GGA
- RMC
- GSV

No external GPS libraries used. PURE MANUAL NMEA parsing.

---

## Features

- UART GPS communication
- Bluetooth Classic telemetry
- Manual NMEA parser
- Latitude/longitude conversion
- Speed conversion (knots → km/h)
- Satellite tracking
- Real-time GPS output
- Supports most UART GPS modules

---

## Hardware

- ESP32
- GPS module
  - NEO-6M
  - NEO-M8N
  - ATGM336H
  - Basically anything outputting NMEA over UART

---

## Wiring

| GPS | ESP32 |
|------|------|
| TX | GPIO16 |
| RX | GPIO17 |
| GND | GND |
| VCC | 3.3V / 5V |

---

## Serial Config

```cpp
#define RXD2 16
#define TXD2 17

GPSSerial.begin(38400, SERIAL_8N1, RXD2, TXD2);
```

## Example Output
```
--- GPS Info ---
Latitude: 40.712776
Longitude: -74.005974
Altitude: 15.20 m
Speed: 32.45 km/h (17.52 kts)
Fix Status: 1
Satellites used: 9

Satellites in view:
PRN:12 Elev:45 Az:180 SNR:38
PRN:15 Elev:52 Az:210 SNR:42
---
```
