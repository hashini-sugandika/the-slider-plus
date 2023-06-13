//########## Basic Setup & Code ##########
//-----------Common for all Module---------
int currentDoorState = 0;       //current status of the door
int lastDoorState = 0;          //last status of the door
char securityMode = 'A';    //select the mode of Multifactor Authentication
char securityModeArray[5];  //Use for converting security types

//-----------LED Blink--------------
int outerLEDPin = 7;
int LedtimeOut = 0;
int ledIntense = 100;

//-----------Touch Sensor Module-----------
const int TouchSensor_Pin = A0;

//-----------Numberpad Module-----------
#include <Keypad.h>
const int ROW_NUM = 4;     //four rows
const int COLUMN_NUM = 4;  //four columns

char keys[ROW_NUM][COLUMN_NUM] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};
byte pin_rows[ROW_NUM] = { 22, 23, 24, 25 };       //connect to the row pinouts of the keypad
byte pin_column[COLUMN_NUM] = { 26, 27, 28, 29 };  //connect to the column pinouts of the keypad
Keypad keypad = Keypad(makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM);

const int passLength = 4;
String password;                   // Define the correct password
char enteredPassword[passLength];  // Variable to store the entered password
int keyIndex = 0;                  // Index to keep track of the key being entered
bool NumpadStatus = false;

//-----------Buzzer Module -----------
const int buzzerPin = 10;  // Pin connected to the buzzer
int volume = 2;

//-----------SD Module----------------
#include <SPI.h>
#include <SD.h>
File myFile;
const int chipSelect = 10;
const int misoPin = 50;  //Same for NFC Module
const int mosiPin = 51;  //Same for NFC Module
const int sckPin = 52;   //Same for NFC Module
#define SD_SS_Pin 49

//-----------Ultrasonic Sensor------------
const int trigPin = 3;              //Trigger pin of the ultrasonic sensor
const int echoPin = 2;              //Echo pin of the ultrasonic sensor
const int thresholdDistance = 100;  // Threshold distance for triggering the door closing in centimeters
int MotionStatus = 0;
int timeOut = 0;  //Motion sensor timeout

//------------Door Sensor------------
const int DOOR_SENSOR_PIN = 13;

//-------------DC Motor Controller----------------
const int motorPwm = 4;  // initializing pin 4 as pwm
const int motorIn_1 = 11;
const int motorIn_2 = 12;
int doorMovementTime = 5000;

//-------------RFID Module-----------
#include <MFRC522.h>
#include <stdio.h>

#define RFID_SS_PIN 48  //slave select pin
#define RST_PIN 5       //reset pin
bool RFIDStatus = false;

MFRC522 mfrc522(RFID_SS_PIN, RST_PIN);  // instatiate a MFRC522 reader object.
MFRC522::MIFARE_Key key;                //create a MIFARE_Key struct named 'key', which will hold the card information

String rfidCard;
int block = 2;  //this is the block number we will write into and then read. Do not write into 'sector trailer' block, since this can make the block unusable.

byte blockcontent[16] = { "DigitalControl__" };  //an array with 16 bytes to be written into one of the 64 card blocks is defined
//byte blockcontent[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};//all zeros. This can be used to delete a block.
byte readbackblock[18];  //This array is used for reading out a block. The MIFARE_Read method requires a buffer that is at least 18 bytes to hold the 16 bytes of a block.

//-------------Relay Module-----------
const int relayPin = 6;  //use for switching between SD module and NFC module

//-------------Fingerprint Module--------
bool FingerprintStatus = false;


//########## Setup & Code ##########
void setup() {
  //setting SPI and SS Pins
  Serial.begin(9600);
  SPI.begin();  // Init SPI bus
  pinMode(outerLEDPin, OUTPUT);

  //Buzzer Module
  pinMode(buzzerPin, OUTPUT);

  //Touch Sensor Module
  pinMode(TouchSensor_Pin, INPUT);

  //Motion Sensor Module
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  //Door Sensor Module
  pinMode(DOOR_SENSOR_PIN, INPUT_PULLUP);
  currentDoorState = digitalRead(DOOR_SENSOR_PIN);

  //Motor controler module
  pinMode(motorPwm, OUTPUT);   // we have to set PWM pin as output
  pinMode(motorIn_1, OUTPUT);  // Logic pins are also set as output
  pinMode(motorIn_2, OUTPUT);

  //SD module
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);
  if (!SD.begin(SD_SS_Pin)) {
    Serial.println("SD card initialization failed!");
  } else {
    Serial.println("SD card initialization Successfull!");
  }
  //Reading from the config file
  ReadSDCard();
  //Write to card
  //WriteSDCard("config.txt", "E,6666");      //refer the end of the code for configuration
  delay(1000);
  digitalWrite(relayPin, HIGH);
  delay(50);

  //NFC reader module
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;  //keyByte is defined in the "MIFARE_Key" 'struct' definition in the .h file of the library
  }
}

void loop() {
  //Choosing security methods depend on the mode
  if (currentDoorState == 0) {
    //Serial.println(securityMode);
    //Serial.println(password);
    switch (securityMode) {
      case 'A':
        Serial.println("Security Mode : Any");
        if (NumPadRead() || RFIDRead() || FingerprintRead()) {
          openDoor();
        }
        break;
      case 'B':
        Serial.println("Security Mode : Numberpad only");
        if (NumPadRead()) {
          openDoor();
        }
        break;
      case 'C':
        Serial.println("Security Mode : NFC only");
        if (RFIDRead()) {
          openDoor();
        }
        break;
      case 'D':
        Serial.println("Security Mode : Fingerprint only");
        if (FingerprintRead()) {
          openDoor();
        }
        break;
      case 'E':
        Serial.println("Security Mode : Numberpad & NFC");
        if ((NumPadRead() || NumpadStatus) && (RFIDRead() || RFIDStatus)) {
          openDoor();
        }
        break;
      case 'F':
        Serial.println("Security Mode : Numberpad & Fingerprint");
        if ((NumPadRead() || NumpadStatus) && (FingerprintRead() || FingerprintStatus)) {
          openDoor();
        }
        break;
      case 'G':
        Serial.println("Security Mode : NFC & Fingerprint");
        if ((RFIDRead() || RFIDStatus) && (FingerprintRead() || FingerprintStatus)) {
          openDoor();
        }
        break;
      case 'H':
        Serial.println("Security Mode : Numberpad & NFC & Fingerprint");
        if ((NumPadRead() || NumpadStatus) && (RFIDRead() || RFIDStatus) && (FingerprintRead() || FingerprintStatus)) {
          openDoor();
          break;
        }
      default:
        break;
    }
  }
  //printing current door status
  Serial.print("Door Status : ");
  Serial.println(ReadDoorSens());

  // Configuring how to function the inside touch button
  if (ReadTouchSens() && currentDoorState == 1) {
    closeDoor();
  } else if (ReadTouchSens() && currentDoorState == 0) {
    openDoor();
  }
  delay(50);

  //automatically closing the door after few time
  if (currentDoorState == 1 && ReadMotionSens() == 0) {
    Serial.println(timeOut++);
    if (timeOut > 100 && MotionStatus == 0) {
      closeDoor();
      MotionStatus = 1;
    }
  }

  //lightup the keypad if someone near the door
  if(ReadMotionSens()){
    analogWrite(outerLEDPin, ledIntense);
    LedtimeOut = 0;

  }else {
    Serial.println(LedtimeOut++);
  }
  if (LedtimeOut > 50){
    analogWrite(outerLEDPin, 0);
  }
}

//########## Functions ##########
void openDoor() {
  WriteToBuzzer(1);
  ControlDoor(1);
  Serial.println("Door is opening...");
  RFIDStatus = false;
  NumpadStatus = false;
  FingerprintStatus = false;
}
void closeDoor() {
  WriteToBuzzer(1);
  ControlDoor(0);
  Serial.println("Door is closing...");
}


//########## Front-end Modules ##########
//------Authentication Modules-----
//RFID Sensor reading
bool RFIDRead() {
  mfrc522.PCD_Init();  // Init MFRC522 card (in case you wonder what PCD means: proximity coupling device)
  //Serial.println("Scan a MIFARE Classic card");

  // Look for new cards (in case you wonder what PICC means: proximity integrated circuit card)
  if (!mfrc522.PICC_IsNewCardPresent()) {  //if PICC_IsNewCardPresent returns 1, a new card has been found and we continue
    return 0;                              //if it did not find a new card is returns a '0' and we return to the start of the loop
  }
  // Select one of the cards
  if (!mfrc522.PICC_ReadCardSerial()) {  //if PICC_ReadCardSerial returns 1, the "uid" struct (see MFRC522.h lines 238-45)) contains the ID of the read card.
    return 0;                            //if it returns a '0' something went wrong and we return to the start of the loop
  }

  //Serial.println("card selected");

  /*****************************************writing and reading a block on the card**********************************************************************/

  //mfrc522.PICC_DumpToSerial(&(mfrc522.uid));//uncomment this if you want to see the entire 1k memory with the block written into it.

  readBlock(block, readbackblock);  //read the block back
  //Serial.print("read block: ");
  char pass[16];
  for (int j = 0; j < 16; j++)  //print the block contents
  {
    //Serial.write(readbackblock[j]);
    pass[j] = readbackblock[j];
    //Serial.write() transmits the ASCII numbers as human readable characters to serial monitor
  }
  //Serial.print("");

  char code[] = { 'D', 'i', 'G', 'i', 't', 'a', 'l', 'C', 'o', 'n', 't', 'r', 'o', 'l', '_', '_' };
  bool flag = false;
  for (int i = 1; i < 16; i++) {
    if (code[i] == pass[i]) {
      flag = true;
    }
  }
  if (flag == true) {
    Serial.println("Access Granted");
    RFIDStatus = true;
    Serial.println("RFID Correct!");
    return 1;
  } else {
    Serial.println("Access Denied");
    WriteToBuzzer(0);
    RFIDStatus = false;
    NumpadStatus = false;
    FingerprintStatus = false;
    return 0;
  }

  //Reset varibles
  flag = 0;
  for (int i = 0; i < sizeof(readbackblock); i++) {
    readbackblock[i] = '\0';
  }
  return 0;
}


int readBlock(int blockNumber, byte arrayAddress[]) {
  int largestModulo4Number = blockNumber / 4 * 4;
  int trailerBlock = largestModulo4Number + 3;  //determine trailer block for the sector

  /*****************************************authentication of the desired block for access***********************************************************/
  byte status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));

  if (status != MFRC522::STATUS_OK) {
    Serial.print("PCD_Authenticate() failed (read): ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return 0;  //return "3" as error message
  }

  /*****************************************reading a block***********************************************************/

  byte buffersize = 18;                                                  //we need to define a variable with the read buffer size, since the MIFARE_Read method below needs a pointer to the variable that contains the size...
  status = mfrc522.MIFARE_Read(blockNumber, arrayAddress, &buffersize);  //&buffersize is a pointer to the buffersize variable; MIFARE_Read requires a pointer instead of just a number
  if (status != MFRC522::STATUS_OK) {
    Serial.print("MIFARE_read() failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return 0;  //return "4" as error message
  }
  //Serial.println("block was read");
}

//NumberPad reading
bool NumPadRead() {
  bool correctPassword = false;
  char key = keypad.getKey();
  // Ignore any non-digit characters
  if (isdigit(key)) {
    enteredPassword[keyIndex] = key;
    keyIndex++;
    Serial.print(key);

    // Check if all digits have been entered
    if (keyIndex == passLength) {
      for (int i = 0; i < passLength; i++) {
        if (enteredPassword[i] == password[i]) {
          correctPassword = true;
        } else {
          correctPassword = false;
          break;
        }
      }
      // Reset variables for the next password entry
      keyIndex = 0;
      memset(enteredPassword, 0, sizeof(enteredPassword));

      if (correctPassword) {
        Serial.println("Correct password entered.");
        NumpadStatus = true;
        Serial.println("NumberPad Correct!");
        return 1;
      } else {
        Serial.println("Wrong password entered.");
        WriteToBuzzer(0);
        RFIDStatus = false;
        NumpadStatus = false;
        FingerprintStatus = false;
        return 0;
      }
    }
  }
  return false;
}

//Fingerprint reading
bool FingerprintRead() {
}

//--------Other Modules-------
//Closing the door when someone leaves the room using motion sensor
int ReadMotionSens() {
  //ultrasonic pulse
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Measure the duration of the echo pulse
  long duration = pulseIn(echoPin, HIGH);

  //In centimeters
  float distance = duration * 0.034 / 2;
  //Serial.println(distance);

  if (distance < thresholdDistance) {
    Serial.println("Object near the door");
    timeOut = 0;
    return 1;
    // Code to keep the door open
  } else {
    Serial.println("Object has passed 100cm from the door");
    return 0;
    // Code to close the door
  }
  delay(1000);
}

//Buzzer write when opening and closing the door
void WriteToBuzzer(bool inputValue) {
  if (inputValue == HIGH) {
    // Number 1 is entered, sound the buzzer
    analogWrite(buzzerPin, volume);
    delay(1000);  // Buzz for 1 second
    analogWrite(buzzerPin, 0);
  }
  if (inputValue == LOW) {
    analogWrite(buzzerPin, volume - 20 < 0 ? 2 : volume - 20);
    delay(100);
    analogWrite(buzzerPin, 0);
    delay(100);
    analogWrite(buzzerPin, volume - 20 < 0 ? 2 : volume - 20);
    delay(100);
    analogWrite(buzzerPin, 0);
  }
}


//########## Back-end Modules ##########
//Opening Door from the inside using touch sensor
bool ReadTouchSens() {
  bool TouchState = digitalRead(TouchSensor_Pin);  //read from the sensor and assign the value to the variable
  return TouchState;
  delay(10);
}

//Reading configurations from the SD Card
void ReadSDCard() {
  const char* filename = "config.txt";
  File file = SD.open(filename);
  String line;
  if (file) {
    while (file.available()) {
      char x = file.read();
      if (x == '\n') {
        break;
      }
      line = line + x;
    }
    char i;
    String config[5];
    for (i = 0; i < line.length(); i++) {
      char c = line.charAt(i);
      if (c == ',') {
        config[0] = line.charAt(i - 1);
        config[1] = line.substring(i + 1);
      }
    }
    config[0].toCharArray(securityModeArray, 5);  //converting string to char
    securityMode = securityModeArray[0];          //setting the security mode
    password = config[1];                         //setting the new password
    file.close();
  } else {
    Serial.println("Error opening file.");
  }
}

//Writing configurations to the SD Card
void WriteSDCard(const char* filename, const char* data) {
  //delete the current file and write
  if(SD.remove(filename)){
     Serial.println("Data written to file.");
  }

  //Writing to the file
  File file = SD.open(filename, FILE_WRITE);

  if (file) {
    file.println(data);
    file.close();
    Serial.println("Data written to file.");
  } else {
    Serial.println("Error opening file.");
  }
}

//Reading the Door sensor to verify door status
bool ReadDoorSens() {
  lastDoorState = currentDoorState;
  currentDoorState = digitalRead(DOOR_SENSOR_PIN);
  delay(50);
  if (lastDoorState == LOW && currentDoorState == HIGH) {
    Serial.println("The door-opening event is detected");
    return 1;
  } else
    delay(50);
  if (lastDoorState == HIGH && currentDoorState == LOW) {
    Serial.println("The door-closing event is detected");
    timeOut = 0;
    return 0;
  }
}

//Controlling the door using the motor
void ControlDoor(bool direction) {
  if (direction == HIGH) {
    // turn clockwise for 3 secs
    Serial.println("Turning Right...");
    digitalWrite(motorIn_1, HIGH);
    digitalWrite(motorIn_2, LOW);
    analogWrite(motorPwm, 255);
    delay(doorMovementTime);
  } else if (direction == LOW) {
    // turn anti clockwise for 3 secs
    Serial.println("Turning Left...");
    digitalWrite(motorIn_1, LOW);
    digitalWrite(motorIn_2, HIGH);
    delay(doorMovementTime);
  }
  //break the motor
  digitalWrite(motorIn_1, HIGH);
  digitalWrite(motorIn_2, HIGH);
  delay(1000);
}





/*
Syntax: 
	<Security Mode>,<Password (4 Digit)>


Meaning of the selected Mode
{
	mode : A
	modules : NumberPad/NFC/FingerPrint
}
{
	mode : B
	modules : NumberPad Only
}
{
	mode : C
	modules : NFC Only
}
{
	mode : D
	modules : FingerPrint Only
}
{
	mode : E
	modules : Require both Numberpad & NFC
}
{
	mode : F
	modules : Require both Numberpad & Fingerprint
}
{
	mode : G
	modules : Require both NFC & Fingerprint
}
{
	mode : H
	modules : Require all Numberpad & NFC & Fingerprint
}
*/
