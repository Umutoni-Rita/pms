#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN 9
#define SS_PIN 10

MFRC522 rfid(SS_PIN, RST_PIN); // Create MFRC522 instance
MFRC522::MIFARE_Key key;

// Data to write
String licensePlate = "RAE713S"; // License plate
String cashAmount = "100000";    // Cash amount

void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();
  Serial.println("Scan the RFID card to write data...");

  // Prepare the default key (0xFF 0xFF 0xFF 0xFF 0xFF 0xFF)
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
}

void loop() {
  // Check for a new card
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    return;
  }

  // Authenticate for sector 1 (blocks 4-7) using key A
  byte sector = 1;
  byte blockAddr = 4; // Starting block for sector 1
  MFRC522::StatusCode status = rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockAddr, &key, &(rfid.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Authentication failed: ");
    Serial.println(rfid.GetStatusCodeName(status));
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
    return;
  }

  // Prepare data for block 4 (license plate)
  byte buffer[18]; // 16 bytes data + 2 bytes CRC
  byte size = 16;
  for (byte i = 0; i < size; i++) {
    buffer[i] = (i < licensePlate.length()) ? licensePlate[i] : ' '; // Pad with spaces
  }

  // Write to block 4
  Serial.print("Writing license plate to block 4... ");
  status = rfid.MIFARE_Write(blockAddr, buffer, size);
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Write failed: ");
    Serial.println(rfid.GetStatusCodeName(status));
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
    return;
  }
  Serial.println("Done");

  // Prepare data for block 5 (cash amount)
  blockAddr = 5;
  for (byte i = 0; i < size; i++) {
    buffer[i] = (i < cashAmount.length()) ? cashAmount[i] : ' '; // Pad with spaces
  }

  // Write to block 5
  Serial.print("Writing cash amount to block 5... ");
  status = rfid.MIFARE_Write(blockAddr, buffer, size);
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Write failed: ");
    Serial.println(rfid.GetStatusCodeName(status));
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
    return;
  }
  Serial.println("Done");

  // Halt PICC and stop encryption
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  delay(1000);
}
