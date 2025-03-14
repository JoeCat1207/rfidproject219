const int rst = 9;    //rfid
const int miso = 12;  //rfid
const int mosi = 11; //rfid
const int sck = 13; //rfid
const int sda = 10; //rfid
const int LEDred = 2; //Red LED
const int LEDgreen = 4; //Green LED
const int LEDyellow = 6; //Yellow LED
const int buzzpin = 7; //buzzer 
const int RX_Pin = 8; //bluetooth
const int TX_Pin = 3; //bluetooth
const int MAX_STRING_LENGTH = 10; // Maximum length for received string
char toothString[MAX_STRING_LENGTH + 1]; // +1 for null terminator
int stringIndex = 0; // Index to track position in the string
#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>
#include <SoftwareSerial.h>

SoftwareSerial tooth(TX_Pin, RX_Pin); //Bluetooth module setup
MFRC522 mfrc522(sda, rst); // Create MFRC522 instance
Servo myServo; //creates servo instance
int angle; // creates servo angle integer

// the authorizedUID is basically just what your card is... I set it to the UID of my blue tag.
byte authorizedUID[4] = {0x68, 0x81, 0x5F, 0x35};

void setup() {
  // Initialize serial comms
  Serial.begin(115200); //seperates actual serial from bluetooth serial
  tooth.begin(9600); // Initialize Bluetooth serial
  
  // Clear the string buffer
  clearToothString();

  //set servo pin
  myServo.attach(8);
  
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
  angle = 0;

  // Check for Bluetooth data
  if (tooth.available() > 0) {
    char inChar = tooth.read();
    
    // If newline or carriage return, process the complete string
    if (inChar == '\n' || inChar == '\r') {
      toothString[stringIndex] = '\0'; // Null terminate the string
      tooth.print("Received: ");
      tooth.println(toothString);
      
      // Check if the string is "yes"
      if (strcmp(toothString, "yes") == 0) {
        // Turn on all LEDs
        digitalWrite(LEDred, HIGH);
        digitalWrite(LEDgreen, HIGH);
        digitalWrite(LEDyellow, HIGH);
        
        // Turn on buzzer for 300ms
        digitalWrite(buzzpin, HIGH);
        delay(300);
        digitalWrite(buzzpin, LOW);
        
        // Keep LEDs on for remainder of 2 seconds
        delay(1700);
        
        // Turn off all LEDs
        digitalWrite(LEDred, LOW);
        digitalWrite(LEDgreen, LOW);
        digitalWrite(LEDyellow, LOW);
      }
      
      // Reset for next string
      clearToothString();
    } 
    // Otherwise add character to the string if there's room
    else if (stringIndex < MAX_STRING_LENGTH) {
      toothString[stringIndex] = inChar;
      stringIndex++;
    }
  }
  

  // Look for new cards
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Select one of the cards
  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  // Show UID on serial (Deprecated as of now to clear up serial)
  //Serial.print(F("RFID Tag UID:"));
  //printHex(mfrc522.uid.uidByte, mfrc522.uid.size);
  //Serial.println("");
  
  // Check if the scanned card matches the authorizedUID
  if (compareUID(mfrc522.uid.uidByte, authorizedUID, mfrc522.uid.size)) {
    //deprecated as of now to clear serial //Serial.println(F("Right card detected!")); 
    
    // Turn on all LEDs
    digitalWrite(LEDred, HIGH);
    digitalWrite(LEDgreen, HIGH); //All LED on
    digitalWrite(LEDyellow, HIGH);
    digitalWrite(buzzpin,LOW); //redundency
    // Wait for 500
    delay(500);
    
    // Turn off all LEDs
    digitalWrite(LEDred, LOW);
    digitalWrite(LEDgreen, LOW);
    digitalWrite(LEDyellow, LOW);
  } else {
    // deprecated to clear serial: //Serial.println(F("Unknown card")); //if it doesnt match authUID
    digitalWrite(buzzpin, HIGH); //buzzer goes off
    delay(300);
    digitalWrite(buzzpin, LOW);
  }

  mfrc522.PICC_HaltA(); // Halt PICC..
  mfrc522.PCD_StopCrypto1(); // Stop encryption..
  
  // Small delay before next read
  delay(200);
}

void printHex(byte *buffer, byte bufferSize) { //calculations for UID 
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

// Function to clear the received string
void clearToothString() {
  memset(toothString, 0, MAX_STRING_LENGTH + 1);
  stringIndex = 0;
}