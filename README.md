# IoT RFID Architecture - Team Shield

Efficient bridging of RFID hardware with a web dashboard using MQTT and WebSockets.

## Team Information
- **Team ID**: `iot_shield_2026`

## MQTT Topics
- `rfid/iot_shield_2026/card/status`: ESP8266 publishes card UID and balance when detected.
- `rfid/iot_shield_2026/card/topup`: Backend publishes top-up commands.
- `rfid/iot_shield_2026/card/balance`: ESP8266 publishes confirmation of balance update.
- `rfid/iot_shield_2026/device/status`: MQTT Last Will (online/offline).
- `rfid/iot_shield_2026/device/health`: Periodic health metrics (IP, RSSI, Memory).

## HTTP API Endpoints
- `GET /balance/:uid`: Retrieve the current balance for a specific card.
- `POST /topup`: Send a top-up command (`uid`, `amount`) to the MQTT broker.

## Hardware Connections (ESP8266 + RC522)
| RC522 Pin | ESP8266 Pin (NodeMCU) | Function |
|-----------|-----------------------|----------|
| 3.3V      | 3V3                   | Power    |
| RST       | D3 (GPIO0)            | Reset    |
| GND       | GND                   | Ground   |
| MISO      | D6 (GPIO12)           | SPI MISO |
| MOSI      | D7 (GPIO13)           | SPI MOSI |
| SCK       | D5 (GPIO14)           | SPI Clock|
| SDA (SS)  | D4 (GPIO2)            | SPI SS   |

## Getting Started

### Backend
1. Navigate to `/backend`.
2. Install dependencies: `npm install`.
3. Start the server: `node server.js`.
4. The server runs on `http://localhost:5000`.

### Firmware
1. Open `/firmware/iot_rfid_project/iot_rfid_project.ino` in Arduino IDE.
2. Update `ssid` and `password` with your WiFi credentials.
3. Install `MFRC522`, `PubSubClient`, and `ArduinoJson` libraries.
4. Upload to ESP8266.

### Frontend
1. Open `/frontend/index.html` in a web browser.
2. Ensure the backend server is running for real-time updates.
