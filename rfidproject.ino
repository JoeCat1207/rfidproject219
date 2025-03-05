const int rst = 9;
const int miso = 12;
const int mosi = 11;
const int sck = 13;
const int sda = 10;
const int LEDred = 2;
const int LEDgreen = 4;
const int LEDyellow = 6;
const int buzzpin = 7;
#include <SPI.h>
#include <MFRC522.h>

MFRC522 mfrc522(sda, rst); // Create MFRC522 instance

// the authorizedUID is basically just what your card is... I set it to the UID of my blue tag.
byte authorizedUID[4] = {0x68, 0x81, 0x5F, 0x35};

void setup() {
  // Initialize serial comms
  Serial.begin(9600);
  
  // Initialize LEDs as outputs
  pinMode(LEDred, OUTPUT);
  pinMode(LEDgreen, OUTPUT);
  pinMode(LEDyellow, OUTPUT);
  pinMode(buzzpin, OUTPUT);
  
  // Initialize SPI
  SPI.begin();
  
  // Initialize card reader
  mfrc522.PCD_Init();
  
  Serial.println(F("RFID reader up. Waiting for card..."));
  
  // Make sure LEDs are off at startup
  digitalWrite(LEDred, LOW);
  digitalWrite(LEDgreen, LOW);
  digitalWrite(LEDyellow, LOW);
  digitalWrite(buzzpin, LOW);
}

void loop() {
  // Look for new cards
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Select one of the cards
  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  // Show UID on serial 
  Serial.print(F("RFID Tag UID:"));
  printHex(mfrc522.uid.uidByte, mfrc522.uid.size);
  Serial.println("");
  
  // Check if the scanned card matches the authorizedUID
  if (compareUID(mfrc522.uid.uidByte, authorizedUID, mfrc522.uid.size)) {
    Serial.println(F("Right card detected!"));
    
    // Turn on all LEDs
    digitalWrite(LEDred, HIGH);
    digitalWrite(LEDgreen, HIGH);
    digitalWrite(LEDyellow, HIGH);
    digitalWrite(buzzpin, HIGH);
    delay(200);
    digitalWrite(buzzpin,LOW);
    // Wait for 500
    delay(500);
    
    // Turn off all LEDs
    digitalWrite(LEDred, LOW);
    digitalWrite(LEDgreen, LOW);
    digitalWrite(LEDyellow, LOW);
  } else {
    Serial.println(F("Unknown card")); //if it doesnt match authUID

  }

  mfrc522.PICC_HaltA(); // Halt PICC..
  mfrc522.PCD_StopCrypto1(); // Stop encryption..
  
  // Small delay before next read
  delay(200);
}

void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

// Function to compare UIDs from Bradford's code
bool compareUID(byte *buffer1, byte *buffer2, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    if (buffer1[i] != buffer2[i]) {
      return false;
    }
  }
  return true;
}