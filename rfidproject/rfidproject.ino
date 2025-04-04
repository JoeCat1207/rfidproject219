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
//#include <MFRC522.h>
#include <Servo.h>
#include <SoftwareSerial.h>


SoftwareSerial tooth(TX_Pin, RX_Pin); //Bluetooth module setup
//MFRC522 mfrc522(sda, rst); // Create MFRC522 instance
Servo myServo; //creates servo instance
int angle; // creates servo angle integer

// the authorizedUID is basically just what your card is... I set it to the UID of my blue tag.
byte authorizedUID[4] = {0x68, 0x81, 0x5F, 0x35};

void setup() {
  // Initialize serial comms
  Serial.begin(9600); //seperates actual serial from bluetooth serial
  tooth.begin(115200); // Initialize Bluetooth serial
  
  // Clear the string buffer

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
  //mfrc522.PCD_Init();
  
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
      
      // Check if the string is "y" instead of "yes"
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
  
  // RFID section is commented out
  // Look for new cards
  //if (!mfrc522.PICC_IsNewCardPresent()) {
  //  return;
  //}

  // Select one of the cards
  //if (!mfrc522.PICC_ReadCardSerial()) {
  //  return;
  //}
}

// Function to clear the received string
void clearToothString() {
  memset(toothString, 0, MAX_STRING_LENGTH + 1);
  stringIndex = 0;
}