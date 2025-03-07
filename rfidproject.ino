const int rst = 9;    //rfid
const int miso = 12;  //rfid
const int mosi = 11; //rfid
const int sck = 13; //rfid
const int sda = 10; //rfid
const int LEDred = 2; //Red LED
const int LEDgreen = 4; //Green LED
const int LEDyellow = 6; //Yellow LED
const int buzzpin = 7; //buzzer 
const int RX_Pin = A0; //bluetooth
const int TX_Pin = A1; //bluetooth
char toothchar = 2;//bluetooth module character memory
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
  Serial.begin(9600);
  Serial.begin(115200);

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

  if (tooth.available() >0){
    toothchar = tooth.read();
    tooth.print("reading new input");
    tooth.println(toothchar);

  }

  if (toothchar == 'y'){
    digitalWrite(buzzpin, LOW);
    myServo.write(angle);
  }

  toothchar = angle;
  

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