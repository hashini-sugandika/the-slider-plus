//########## Basic Setup & Code ##########
//-----------Common for all Module---------
bool currentDoorState;   //current status of the door
bool lastDoorState;      //last status of the door
char securityMode = 'A';    //select the mode of Multifactor Authentication
char securityModeArray[5];  //Use for converting security types

//-----------LIghtup Numberpad--------------
int outerLEDPin = 7;
int LedtimeOut = 0;
int ledIntense = 100;  //Intese of tge light

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

const int passLength = 4;          //password length
String password;                   // Define the correct password
char enteredPassword[passLength];  // Variable to store the entered password
int keyIndex = 0;                  // Index to keep track of the key being entered
bool NumpadStatus = false;         //current numberpad unlock status

//-----------Buzzer Module -----------
const int buzzerPin = 10;  // Pin connected to the buzzer
int volume = 255;          //volume of the buzzer

//-----------SD Module----------------
#include <SPI.h>
#include <SD.h>
File myFile;  //creating a file instance
const int chipSelect = 10;
const int misoPin = 50;  //Same for NFC Module
const int mosiPin = 51;  //Same for NFC Module
const int sckPin = 52;   //Same for NFC Module
#define SD_SS_Pin 49     //Select slave pin for SD Module

//-----------Ultrasonic Sensor------------
const int trigPin = 3;              //Trigger pin of the ultrasonic sensor
const int echoPin = 2;              //Echo pin of the ultrasonic sensor
const int thresholdDistance = 100;  // Threshold distance for triggering the door closing in centimeters
int MotionStatus = 0;               //Whether the sensor is triggered or not
int timeOut = 0;                    //Motion sensor timeout
//------------Door Sensor------------
const int DOOR_SENSOR_PIN = 13;  //door sensor pin

//-------------DC Motor Controller----------------
const int motorPwm = 9;  // initializing pin 4 as pwm
const int motorIn_1 = 11;
const int motorIn_2 = 12;
int motorSpeed = 100;        //speed of the motor
//int doorMovementTime = 50;  //control the angle of motor turn

//-------------RFID Module-----------
#include <MFRC522.h>
#include <stdio.h>

#define RFID_SS_PIN 48    //slave select pin for RFID module
#define RST_PIN 5         //reset pin
bool RFIDStatus = false;  //current read of the rfid module

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
bool FingerprintStatus = false;  //current status of the fingerprint module

//--------------IR Sensor Module-----------
int openDoorpin = A1;  //when door is open
int openDoorRead = 0;
int closeDoorpin = A2;  //when door is close
int closeDoorRead = 0;


//########## Setup & Code ##########
void setup() {
  //setting SPI and SS Pins
  Serial.begin(9600);
  SPI.begin();  // Init SPI bus
  pinMode(outerLEDPin, OUTPUT);

  //Buzzer Module
  pinMode(buzzerPin, OUTPUT);

  //IR modules
  pinMode(openDoorpin, INPUT);
  pinMode(closeDoorpin, INPUT);

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
  digitalWrite(relayPin, LOW);  //turn on the replay to read from the SD module
  if (!SD.begin(SD_SS_Pin)) {   //initialize the SD module to read or write
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
    switch (securityMode) {  //selecting security mode based on the configured in the text file
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
  analogWrite(motorPwm, motorSpeed);  //controlling the speed of the door

  // Configuring how to function the inside touch button
  if (ReadTouchSens() && currentDoorState == 1) {  //close the door of door is open
    closeDoor();
  } else if (ReadTouchSens() && currentDoorState == 0) {  //open the door if door is close
    openDoor();
  }
  delay(10);

  //automatically closing the door after few time
  if (currentDoorState == 1 && ReadMotionSens() == 0) {
    Serial.println(timeOut++);
    if (timeOut > 100 && MotionStatus == 0) {
      closeDoor();
      MotionStatus = 1;
    }
  }

  //lightup the keypad if someone near the door
  if (ReadMotionSens()) {
    analogWrite(outerLEDPin, ledIntense);
    LedtimeOut = 0;
  } else {
    Serial.println(LedtimeOut++);
  }
  if (LedtimeOut > 50) {
    analogWrite(outerLEDPin, 0);
  }
}

//########## Functions ##########
void openDoor() {  //open the door function
  WriteToBuzzer(1);
  while (!openDoorSens()) {  //looping until fully openning the door
    ControlDoor(1);
    if (ReadTouchSens()) {
      break;
    }
  }
  ControlDoor(2);  //breake the motor
  Serial.println("Door is opening...");
  //resetting the status of each module
  RFIDStatus = false;
  NumpadStatus = false;
  FingerprintStatus = false;
}
void closeDoor() {  //closing the door function
  WriteToBuzzer(1);
  while (ReadDoorSens()) {  //looping until fully closing the door
    ControlDoor(0);
    if (ReadTouchSens()) {
      break;
    }
  }
  ControlDoor(2);  //brake the motor
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

  char code[] = { 'D', 'i', 'G', 'i', 't', 'a', 'l', 'C', 'o', 'n', 't', 'r', 'o', 'l', '_', '_' };  //password of the RFID card to check
  bool flag = false;
  for (int i = 1; i < 16; i++) {
    if (code[i] == pass[i]) {
      flag = true;
    }
  }
  if (flag == true) {  //checking whether the card is correct or not
    Serial.println("Access Granted");
    RFIDStatus = true;
    Serial.println("RFID Correct!");
    return 1;
  } else {
    Serial.println("Access Denied");
    WriteToBuzzer(0);
    //resetting the status of modules
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

//reading from the RFID card
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
  char key = keypad.getKey();  //reading from the num pad
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

      //checking whether the entered password is correct or not
      if (correctPassword) {
        Serial.println("Correct password entered.");
        NumpadStatus = true;
        Serial.println("NumberPad Correct!");
        return 1;
      } else {
        Serial.println("Wrong password entered.");
        WriteToBuzzer(0);
        //resetting the module status
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
  //fingerprint module can be added here
  //just return 1 and 0 based on success or failure of the reading
  //add other codes to the relevent parts
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

  // Code to keep the door open
  if (distance < thresholdDistance) {
    Serial.println("Object near the door");
    timeOut = 0;
    return 1;
  } else {
    // Code to close the door
    Serial.println("Object has passed 100cm from the door");
    MotionStatus == 0;
    return 0;
  }
}

//Buzzer write when opening and closing the door
void WriteToBuzzer(bool inputValue) {
  if (inputValue == HIGH) {
    // Number 1 is entered, sound the buzzer
    analogWrite(buzzerPin, volume);
    delay(1000);  // Buzz for 1 second
    analogWrite(buzzerPin, 0);
  }
  // buzzer for wrong inputs
  if (inputValue == LOW) {
    analogWrite(buzzerPin, volume - 20 < 0 ? volume : volume - 20);
    delay(100);
    analogWrite(buzzerPin, 0);
    delay(100);
    analogWrite(buzzerPin, volume - 20 < 0 ? volume : volume - 20);
    delay(100);
    analogWrite(buzzerPin, 0);
  }
}


//########## Back-end Modules ##########
//Opening Door from the inside using touch sensor
bool ReadTouchSens() {
  bool TouchState = digitalRead(TouchSensor_Pin);  //read from the sensor and assign the value to the variable
  return TouchState;
  delay(5);
}

//IR sensoe Module, Control the door state
bool openDoorSens() {
  openDoorRead = digitalRead(openDoorpin);
  //Serial.println(openDoorRead);
  return openDoorRead;
}

bool closeDoorSens() {
  closeDoorRead = digitalRead(closeDoorpin);
  //Serial.println(closeDoorRead);
  return closeDoorRead;
}


//Reading configurations from the SD Card
void ReadSDCard() {
  const char* filename = "config.txt";
  File file = SD.open(filename);
  String line;
  if (file) {  //checking for ht file
    while (file.available()) {
      char x = file.read();
      if (x == '\n') {  //read the first line
        break;
      }
      line = line + x;  //add each char into a string line
    }
    char i;
    String config[5];  //saving config to an array
    for (i = 0; i < line.length(); i++) {
      char c = line.charAt(i);
      if (c == ',') {  //break into indexes using ","
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
  if (SD.remove(filename)) {
    Serial.println("Data written to file.");
  }

  //Writing to the file
  File file = SD.open(filename, FILE_WRITE);
  //checking for the file and write
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
  currentDoorState = digitalRead(DOOR_SENSOR_PIN);  //reading from the door sensor
  delay(10);
  if (lastDoorState == 0 && currentDoorState == 1) {
    Serial.println("The door-opening event is detected");
    return 1;
  } else
    delay(10);
  if (lastDoorState == 1 && currentDoorState == 0) {
    Serial.println("The door-closing event is detected");
    timeOut = 0;
    return 0;
  }
  //return 0;
}

//Controlling the door using the motor
void ControlDoor(int direction) {
  if (direction == 1) {
    // turn clockwise for 3 secs
    Serial.println("Turning Right...");
    digitalWrite(motorIn_1, HIGH);
    digitalWrite(motorIn_2, LOW);
    //delay(doorMovementTime);
  } else if (direction == 0) {
    // turn anti clockwise for 3 secs
    Serial.println("Turning Left...");
    digitalWrite(motorIn_1, LOW);
    digitalWrite(motorIn_2, HIGH);
    //delay(doorMovementTime);
  } else if (direction = 2) {
    //break the motor
    digitalWrite(motorIn_1, HIGH);
    digitalWrite(motorIn_2, HIGH);
    //delay(doorMovementTime);
  }
}




//Configuration options for the SliderPlus
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
