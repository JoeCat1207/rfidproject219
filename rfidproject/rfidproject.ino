#define RST_PIN   10  
#define SS_PIN    9  
#define MISO_PIN  11 
#define MOSI_PIN  12 
#define SCK_PIN   13 
const int LEDred = 5; //Red LED
const int LEDgreen = 4; //Green LED
const int LEDyellow = 6; //Yellow LED
const int buzzpin = 2; //buzzer 
#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>
Servo myServo;
// Ultrasonic sensor pins
#define TRIG_PIN 7
#define ECHO_PIN 8

// Baseline distance for ultrasonic sensor
long initialDistance = 0;

// Measure distance in cm using ultrasonic sensor
long measureDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH);
  long distance = duration / 58;
  return distance;
}

// Perform the door opening sequence
void openDoor() {
  digitalWrite(LEDgreen, HIGH);
  digitalWrite(buzzpin, HIGH);
  delay(300);
  digitalWrite(buzzpin, LOW);
  myServo.write(0);
  delay(400);
  myServo.write(90);
  delay(2000);
  myServo.write(180);
  delay(300);
  myServo.write(90);
  digitalWrite(LEDgreen, LOW);
}

MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance
int angle; // creates servo angle integer

// the authorizedUID is basically just what your card is... I set it to the UID of my blue tag.
byte authorizedUID[4] = {0x68, 0x81, 0x5F, 0x35};

void setup() {
  // Initialize serial comms
  Serial.begin(9600); // Serial for debugging
  
  // Set servo pin
  myServo.attach(3);  // Servo on pin 3
  myServo.write(90);  // Initialize at neutral position (stop for continuous)
  
  // Initialize LEDs and buzzer as outputs
  pinMode(LEDred, OUTPUT);
  pinMode(LEDgreen, OUTPUT);
  pinMode(LEDyellow, OUTPUT);
  pinMode(buzzpin, OUTPUT);
  // Initialize ultrasonic sensor pins
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  
  // Initialize SPI
  SPI.begin();
  
  // Initialize card reader
  mfrc522.PCD_Init();
  
  // Debug messages
  Serial.println(F("System initialized"));
  Serial.println(F("RFID reader ready"));
  
  // Flash all LEDs to indicate startup
  digitalWrite(LEDred, HIGH);
  digitalWrite(LEDgreen, HIGH);
  digitalWrite(LEDyellow, HIGH);
  delay(300);
  digitalWrite(LEDred, LOW);
  digitalWrite(LEDgreen, LOW);
  digitalWrite(LEDyellow, LOW);
  // Measure and store initial distance
  initialDistance = measureDistance();
  Serial.print(F("Initial distance: "));
  Serial.print(initialDistance);
  Serial.println(F(" cm"));
}

void loop() {
  angle = 0;
  bool rfidDetected = false;
  // Bluetooth section: open door on '1' received
  if (Serial.available() > 0) {
    char c = Serial.read();
    if (c == '1') {
      Serial.println(F("Bluetooth access granted"));
      openDoor();
    }
  }

  // Ultrasonic sensor check: buzz if object moves more than 10cm from initial distance
  long currentDistance = measureDistance();
  long delta = currentDistance - initialDistance;
  // if moved farther or closer by more than 10cm
  if (delta > 10 || delta < -10) {
    digitalWrite(buzzpin, HIGH);
    delay(200);
    digitalWrite(buzzpin, LOW);
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
    
    // Compare UID with authorized card
    if (checkUID(mfrc522.uid.uidByte, authorizedUID)) {
      // Access granted
      Serial.println("Access granted!");
      
      // Open door sequence
      openDoor();
    } else {
      // Access denied
      Serial.println("Access denied!");
      
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

// Function to check if the scanned card matches the authorized card
bool checkUID(byte scannedUID[], byte storedUID[]) {
  for (int i = 0; i < 4; i++) {
    if (scannedUID[i] != storedUID[i]) {
      return false;
    }
  }
  return true;
}