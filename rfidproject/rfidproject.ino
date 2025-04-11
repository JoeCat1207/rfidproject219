#define RST_PIN   10  
#define SS_PIN    9  
#define MISO_PIN  11 
#define MOSI_PIN  12 
#define SCK_PIN   13 
const int LEDred = 5; //Red LED
const int LEDgreen = 4; //Green LED
const int LEDyellow = 6; //Yellow LED
const int buzzpin = 2; //buzzer 
const int RX_Pin = 7; //Arduino RX pin for Bluetooth TX
const int TX_Pin = 8; //Arduino TX pin for Bluetooth RX
const int MAX_STRING_LENGTH = 20; // Maximum length for received string
char toothString[MAX_STRING_LENGTH + 1]; // +1 for null terminator
int stringIndex = 0; // Index to track position in the string
unsigned long lastBTCheck = 0; // Last time we checked Bluetooth
#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>
#include <SoftwareSerial.h>


SoftwareSerial BT(RX_Pin, TX_Pin); //Bluetooth module setup
MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance
Servo myServo; //creates servo instance
int angle; // creates servo angle integer

// the authorizedUID is basically just what your card is... I set it to the UID of my blue tag.
byte authorizedUID[4] = {0x68, 0x81, 0x5F, 0x35};

void setup() {
  // Initialize serial comms
  Serial.begin(9600); // Serial for debugging
  BT.begin(9600);     // Bluetooth serial at 9600 baud
  
  // Clear the string buffer
  clearToothString();

  // Set servo pin
  myServo.attach(3);  // Servo on pin 3
  myServo.write(90);  // Initialize at neutral position (stop for continuous)
  
  // Initialize LEDs and buzzer as outputs
  pinMode(LEDred, OUTPUT);
  pinMode(LEDgreen, OUTPUT);
  pinMode(LEDyellow, OUTPUT);
  pinMode(buzzpin, OUTPUT);
  
  // Initialize SPI
  SPI.begin();
  
  // Initialize card reader
  mfrc522.PCD_Init();
  
  // Debug messages
  Serial.println(F("System initialized"));
  Serial.println(F("RFID reader ready"));
  Serial.println(F("Bluetooth ready - send 'open' to grant access"));
  
  // Bluetooth welcome message
  BT.println(F("Bluetooth connected"));
  BT.println(F("Send 'open' to unlock"));
  
  // Flash all LEDs to indicate startup
  digitalWrite(LEDred, HIGH);
  digitalWrite(LEDgreen, HIGH);
  digitalWrite(LEDyellow, HIGH);
  delay(300);
  digitalWrite(LEDred, LOW);
  digitalWrite(LEDgreen, LOW);
  digitalWrite(LEDyellow, LOW);
}

void loop() {
  angle = 0;
  bool rfidDetected = false;

  // Check for Bluetooth data
  while (tooth.available() > 0) {
    char inChar = tooth.read();
    Serial.print("BT received: ");
    Serial.println(inChar);
    
    // If newline or carriage return, process the complete string
    if (inChar == '\n' || inChar == '\r') {
      // Only process if we have some data
      if (stringIndex > 0) {
        toothString[stringIndex] = '\0'; // Null terminate the string
        tooth.print("Received: ");
        tooth.println(toothString);
        Serial.print("Bluetooth command: ");
        Serial.println(toothString);
        
        // Check if the string is "y"
        if (strcmp(toothString, "y") == 0) {
          // Turn on all LEDs
          digitalWrite(LEDred, HIGH);
          digitalWrite(LEDgreen, HIGH);
          digitalWrite(LEDyellow, HIGH);
          
          // Delay for 500ms
          delay(500);
          
          // Turn off all LEDs
          digitalWrite(LEDred, LOW);
          digitalWrite(LEDgreen, LOW);
          digitalWrite(LEDyellow, LOW);
          tooth.println("LED test complete");
        }
        // Check for "yes" command to grant access
        else if (strcmp(toothString, "yes") == 0) {
          // Grant access via Bluetooth command
          Serial.println("Access granted via Bluetooth!");
          tooth.println("Access granted via Bluetooth!");
          
          // Turn on green LED and buzzer
          digitalWrite(LEDgreen, HIGH);
          digitalWrite(buzzpin, HIGH);
          
          // Short buzzer on time
          delay(300);
          digitalWrite(buzzpin, LOW);
          
          // Control 360-degree servo - rotate one direction
          myServo.write(180); // Full speed one direction
          delay(300);         // Run for short time
          
          myServo.write(90);  // Stop the servo
          delay(2000);        // Hold door open
          
          // Rotate in opposite direction
          myServo.write(0);   // Full speed other direction
          delay(400);         // Run for short time
          
          myServo.write(90);  // Stop the servo
          
          // Turn off green LED
          digitalWrite(LEDgreen, LOW);
        }
        else {
          // Unknown command
          tooth.print("Unknown command: ");
          tooth.println(toothString);
          tooth.println("Try 'y' or 'yes'");
        }
        
        // Reset for next string
        clearToothString();
      }
    } 
    // Otherwise add character to the string if there's room
    else if (stringIndex < MAX_STRING_LENGTH) {
      toothString[stringIndex] = inChar;
      stringIndex++;
    }
  }
  
  // RFID section
  // Look for new cards - don't return early
  if (mfrc522.PICC_IsNewCardPresent()) {
    // Select one of the cards - don't return early
    if (mfrc522.PICC_ReadCardSerial()) {
      rfidDetected = true;
    }
  }

  // Process RFID card if detected
  if (rfidDetected) {
    // Print UID of scanned card
    Serial.print("Card UID: ");
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
      Serial.print(mfrc522.uid.uidByte[i], HEX);
    }
    Serial.println();
    
    // Also send UID via Bluetooth
    tooth.print("Card UID: ");
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      tooth.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
      tooth.print(mfrc522.uid.uidByte[i], HEX);
    }
    tooth.println();
    
    // Compare UID with authorized card
    if (checkUID(mfrc522.uid.uidByte, authorizedUID)) {
      // Access granted
      Serial.println("Access granted!");
      tooth.println("Access granted!");
      
      // Turn on green LED and buzzer
      digitalWrite(LEDgreen, HIGH);
      digitalWrite(buzzpin, HIGH);
      
      // Short buzzer on time
      delay(300);
      digitalWrite(buzzpin, LOW);
      
      // Control 360-degree servo - rotate one direction
      myServo.write(180); // Full speed one direction
      delay(300);         // Run for short time
      
      myServo.write(90);  // Stop the servo
      delay(2000);        // Hold door open
      
      // Rotate in opposite direction
      myServo.write(0);   // Full speed other direction
      delay(400);         // Run for short time
      
      myServo.write(90);  // Stop the servo
      
      // Turn off green LED
      digitalWrite(LEDgreen, LOW);
    } else {
      // Access denied
      Serial.println("Access denied!");
      tooth.println("Access denied!");
      
      // Turn on red LED and buzzer
      digitalWrite(LEDred, HIGH);
      digitalWrite(buzzpin, HIGH);
      delay(300); // Shortened buzzer time
      digitalWrite(buzzpin, LOW);
      delay(700); // Keep red LED on a bit longer
      digitalWrite(LEDred, LOW);
    }
    
    // Halt PICC
    mfrc522.PICC_HaltA();
    // Stop encryption on PCD
    mfrc522.PCD_StopCrypto1();
  }
  
  // Small delay to stabilize the loop
  delay(50);
}

// Function to clear the received string
void clearToothString() {
  memset(toothString, 0, MAX_STRING_LENGTH + 1);
  stringIndex = 0;
}

// Function to check if the scanned card matches the authorized card
bool checkUID(byte scannedUID[], byte storedUID[]) {
  for (int i = 0; i < 4; i++) {
    if (scannedUID[i] != storedUID[i]) {
      return false;
    }
  }
  return true;
}

// Grant access function - shared by RFID and Bluetooth
void grantAccess(const char* source) {
  // Display access message
  Serial.print("Access granted via ");
  Serial.println(source);
  
  if (strcmp(source, "RFID") == 0) {
    BT.println("Access granted via RFID");
  } else {
    BT.println("Access granted!");
  }
  
  // Turn on green LED and buzzer
  digitalWrite(LEDgreen, HIGH);
  digitalWrite(buzzpin, HIGH);
  
  // Short buzzer on time
  delay(300);
  digitalWrite(buzzpin, LOW);
  
  // Control 360-degree servo - rotate one direction
  myServo.write(180); // Full speed one direction
  delay(300);         // Run for short time
  
  myServo.write(90);  // Stop the servo
  delay(2000);        // Hold door open
  
  // Rotate in opposite direction
  myServo.write(0);   // Full speed other direction
  delay(400);         // Run for short time
  
  myServo.write(90);  // Stop the servo
  
  // Turn off green LED
  digitalWrite(LEDgreen, LOW);
}