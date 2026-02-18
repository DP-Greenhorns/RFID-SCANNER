# RFID_123

A PlatformIO project for ESP32/ESP32-S3 microcontrollers that communicates with an RFID reader over UART, parses tag responses, and prints tag data (EPC, TID, RSSI) to the serial monitor.

## Features
- UART communication with RFID reader
- CRC-16-CCITT calculation for protocol
- Parses and displays EPC, TID, and RSSI from tag responses
- Designed for ESP32/ESP32-S3 (see board config below)

## Hardware
- ESP32 or ESP32-S3 board (see `platformio.ini`)
- RFID reader module (UART interface)
- Connect RFID TX/RX to ESP32 RX (GPIO 18) / TX (GPIO 17)

## Getting Started

### 1. Clone the repository
```sh
git clone <your-repo-url>
cd RFID_123
```

### 2. Install PlatformIO
- [PlatformIO IDE for VSCode](https://platformio.org/install/ide?install=vscode)
- Or [PlatformIO Core CLI](https://docs.platformio.org/en/latest/core/installation.html)

### 3. Select the correct board
- Default: `esp32s3_8MB` (custom board in `boards/`)
- If you have a regular ESP32, edit `platformio.ini`:
  - Change `board = esp32s3_8MB` to `board = esp32dev` or your ESP32 variant

### 4. Build and upload
Connect your board via USB. Update the upload port if needed (e.g., `COM9`).

```sh
pio run --target upload --upload-port COM9
```

### 5. Monitor serial output
```sh
pio device monitor --baud 115200 --port COM9
```

## File Structure
- `src/main.cpp` — Main application logic
- `platformio.ini` — PlatformIO project config
- `boards/esp32s3_8MB.json` — Custom board definition (ESP32-S3)
- `lib/` — External libraries
- `include/` — Project headers

## Troubleshooting
- **Fatal error: This chip is ESP32, not ESP32-S3**
  - Change the `board` in `platformio.ini` to match your hardware (e.g., `esp32dev` for ESP32).
- **Upload issues**
  - Check your COM port and drivers.
  - Use `pio device list` to find the correct port.

## License
MIT
