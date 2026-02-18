#include <Arduino.h>
#include <HardwareSerial.h>

// ================= PINS & CONFIG =================
#define RX_PIN      18
#define TX_PIN      17
#define BAUDRATE    115200

HardwareSerial RFID(2);

// ================= PROTOCOL CONSTANTS =================
#define SOF_BYTE             0xBB
#define CMD_EXT_INVENTORY    0x21
#define RESP_TAG_NOTIFY      0xE0

#define INV_FLAG_TID_EPC_RSSI  0x0C

// Header Flags
#define HEADER_FLAG_EPC      0x01
#define HEADER_FLAG_TID      0x02
#define HEADER_FLAG_RSSI     0x04

// ================= CRC CALCULATOR =================
// CRC-16-CCITT (0x1021), Init 0xFFFF
uint16_t calculateCRC(uint8_t *data, uint8_t len) {
  uint16_t crc = 0xFFFF;
  for (int i = 0; i < len; i++) {
    crc ^= data[i];
    for (int j = 0; j < 8; j++) {
      if (crc & 0x8000)
        crc = (crc << 1) ^ 0x1021;
      else
        crc <<= 1;
    }
  }
  return ~crc;
}

// ================= SEND COMMAND =================
void sendExtendedInventory() {
  uint8_t frame[6];

  frame[0] = SOF_BYTE;
  frame[1] = 0x02;                    // Cmd + Data length
  frame[2] = CMD_EXT_INVENTORY;       // 0x21
  frame[3] = INV_FLAG_TID_EPC_RSSI;   // Request EPC + TID + RSSI

  uint16_t crc = calculateCRC(&frame[2], 2);
  frame[4] = (crc >> 8) & 0xFF;
  frame[5] = crc & 0xFF;

  while (RFID.available()) RFID.read();
  RFID.write(frame, 6);
}

// ================= PARSE RESPONSE =================
void parseResponse(uint8_t *frame, int len) {
  uint8_t cmd = frame[2];

  if (cmd == RESP_TAG_NOTIFY) { // 0xE0
    int cursor = 3;
    uint8_t headerFlags = frame[cursor++];

    Serial.println("----------------------------------");

    // Device ID (Bit 6)
    if (headerFlags & 0x40) {
      cursor += 4;
    }

    // EPC
    if (headerFlags & HEADER_FLAG_EPC) {
      uint8_t epcLen = frame[cursor++];
      Serial.print("EPC : ");
      for (int i = 0; i < epcLen; i++) {
        Serial.printf("%02X", frame[cursor++]);
      }
      Serial.println();
    }

    // TID
    if (headerFlags & HEADER_FLAG_TID) {
      uint8_t tidLen = frame[cursor++];
      Serial.print("TID : ");
      for (int i = 0; i < tidLen; i++) {
        Serial.printf("%02X", frame[cursor++]);
      }
      Serial.println();
    }

    // User Memory (skip if present)
    if (headerFlags & 0x08) {
      uint8_t memLen = frame[cursor++];
      cursor += memLen;
    }

    // ================= RSSI (FIXED AS PER DOC) =================
    if (headerFlags & HEADER_FLAG_RSSI) {
      uint8_t rssiH = frame[cursor++];
      uint8_t rssiL = frame[cursor++];

      // Combine as signed 16-bit
      int16_t rawRSSI = (int16_t)((rssiH << 8) | rssiL);

      // Divide by 100 as per document
      float rssiDbm = rawRSSI / 100.0;

      Serial.printf("RSSI: %.2f dBm\n", rssiDbm);
    }
  }
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);
  RFID.begin(BAUDRATE, SERIAL_8N1, RX_PIN, TX_PIN);
  delay(1000);
  Serial.println("Reader Initialized. Starting Scan...");
}

// ================= LOOP =================
void loop() {
  sendExtendedInventory();

  unsigned long startT = millis();
  while (millis() - startT < 200) {
    if (RFID.available()) {
      if (RFID.read() == SOF_BYTE) {
        uint8_t len = RFID.read();
        if (len > 64) continue;

        uint8_t buffer[70];
        buffer[0] = SOF_BYTE;
        buffer[1] = len;

        for (int i = 0; i < len + 2; i++) {
          while (!RFID.available()) {
            if (millis() - startT > 200) break;
          }
          buffer[2 + i] = RFID.read();
        }

        parseResponse(buffer, len + 4);
      }
    }
  }

  delay(100);
}