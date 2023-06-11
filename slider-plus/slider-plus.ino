//########## Basic Setup & Code ##########
//-----------Common for all Module---------
int doorStatus = 0; //current status of the door
char securityMode = 'A'; //select the mode of Multifactor Authentication

//-----------Touch Sensor Module-----------
const int TouchSensor_Pin = A0;

//-----------Numberpad Module-----------
#include <Keypad.h>
const int ROW_NUM = 4; //four rows
const int COLUMN_NUM = 4; //four columns

char keys[ROW_NUM][COLUMN_NUM] = {
  {'1','2','3', 'A'},
  {'4','5','6', 'B'},
  {'7','8','9', 'C'},
  {'*','0','#', 'D'}
};
byte pin_rows[ROW_NUM] = {22, 23, 24, 25}; //connect to the row pinouts of the keypad
byte pin_column[COLUMN_NUM] = {26, 27, 28, 29}; //connect to the column pinouts of the keypad
Keypad keypad = Keypad( makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM );

const int passLength = 4;
char password[passLength] = "1234";  // Define the correct password
char enteredPassword[passLength];  // Variable to store the entered password
int keyIndex = 0;  // Index to keep track of the key being entered

//-----------Buzzer Module -----------
const int buzzerPin = 9;    // Pin connected to the buzzer

//-----------SD Module----------------
#include <SPI.h>
#include <SD.h>
File myFile;
const int chipSelect = 10;

//------------Door Sensor------------
const int DOOR_SENSOR_PIN = 13;
int currentDoorState;
int lastDoorState;



//########## Setup & Code ##########
void setup() {
  Serial.begin(9600);

  //Buzzer Module
  pinMode(buzzerPin, OUTPUT);

  //Touch Sensor Module
  pinMode(TouchSensor_Pin, INPUT);

  //SD module
  if (!SD.begin()) {
    Serial.println("initialization failed!");
  }
  
  //Door Sensor Module
  pinMode(DOOR_SENSOR_PIN, INPUT_PULLUP); 
  currentDoorState = digitalRead(DOOR_SENSOR_PIN); 

    //reading from the file and setup password and mode
  Serial.println(ReadSDCard());
}

void loop() {
  // Choosing security methods depend on the mode
  if(doorStatus == 0){
    switch (securityMode){
      case 'A':
        Serial.println("Security Mode : Any");
        if(NumPadRead() || RFIDRead() || FingerprintRead()){
          openDoor();
        }
       break;
      case 'K':
        Serial.println("Security Mode : Numberpad only");
        if(NumPadRead()){
          openDoor();
        }
        break;
      case 'N':
        Serial.println("Security Mode : NFC only");
        if(RFIDRead()){
          openDoor();
        }
        break;
      case 'F':
        Serial.println("Security Mode : Fingerprint only");
        if(FingerprintRead()){
          openDoor();
        }
        break;
      case 'KN':
        Serial.println("Security Mode : Numberpad & NFC");
        if(NumPadRead() && RFIDRead()){
          openDoor();
        }
        break;
      case 'KF':
        Serial.println("Security Mode : Numberpad & Fingerprint");
        if(NumPadRead() && FingerprintRead()){
          openDoor();
        }
        break;
      case 'NF':
        Serial.println("Security Mode : NFC & Fingerprint");
        if(RFIDRead() && FingerprintRead()){
          openDoor();
        }
        break;
      case 'KNF':
        Serial.println("Security Mode : Numberpad & NFC & Fingerprint");
        if(NumPadRead() && RFIDRead() && FingerprintRead()){
          openDoor();
        }
        break;
      default:
        break;
    }
  }

  // Configuring how to function the inside touch button
  if(ReadTouchSens() && doorStatus == 1){
    closeDoor();
  } else if(ReadTouchSens() && doorStatus == 0){
    openDoor();
  }
  delay(100);
}

//########## Functions ##########
void openDoor(){
  doorStatus = 1;
  Serial.println("Door is opening...");
  WriteToBuzzer(1);
}
void closeDoor(){
  doorStatus = 0;
  Serial.println("Door is closing...");
  WriteToBuzzer(1);
}


//########## Front-end Modules ##########
//------Authentication Modules-----
//RFID Sensor reading
bool RFIDRead(){

}

//NumberPad reading
bool NumPadRead(){
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
          return true;
        } else {
          Serial.println("Wrong password entered.");
          return false;
        }
      }
    }
    return false;
}
//Fingerprint reading
bool FingerprintRead(){
}

//--------Other Modules-------
//Closing the door when someone leaves the room using motion sensor
int ReadMotionSens(){
}

//Buzzer write when opening and closing the door
void WriteToBuzzer(bool inputValue){
  if (inputValue == HIGH) {
        // Number 1 is entered, sound the buzzer
        analogWrite(buzzerPin, 2);
        Serial.println("Door is moving...");
        delay(1000);  // Buzz for 1 second
        analogWrite(buzzerPin, 0);
      }
}


//########## Back-end Modules ##########
//Opening Door from the inside using touch sensor
bool ReadTouchSens(){
  bool TouchState = digitalRead(TouchSensor_Pin);   //read from the sensor and assign the value to the variable
  return TouchState;
  delay(10);
}
//Reading configurations from the SD Card
char ReadSDCard(){
  const char filename = "slider-plus-data.txt";
  char fileContent; 
  File file = SD.open(filename);

  if (file) {
    while (file.available()) {
      fileContent = file.read();    //read from the file
    }
    file.close();
    return fileContent;
  } else {
    Serial.println("Error opening file.");
    return "";
  }
}
//Writing configurations to the SD Card
void WriteSDCard(const char* filename, const char* data){
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
void ReadDoorSens(){
  File file = SD.open("ss.txt");
  String line;
  if (file) {
    while (file.available()) {
      char x = file.read();
      if (x == '\n'){
        break;
      }
      line = line + x;      
    }
    char i;
    char type;
    String passWord;
    String config[5];
    for (i = 0; i<line.length(); i++){
      char c = line.charAt(i);
      if (c == ','){
        config[0] = line.charAt(i-1);
        config[1] = line.substring(i+1);
        break;
      }
    }
    Serial.println(config[0]);
    Serial.println(config[1]);  
    file.close();

  } else {
    Serial.println("Error opening file.");
  }
}
//Controlling the door using the motor
void ControlDoor(){
}



/*Meaning of the selected Mode
{
	mode : A
	modules : NumberPad/NFC/FingerPrint
}
{
	mode : K
	modules : NumberPad Only
}
{
	mode : N
	modules : NFC Only
}
{
	mode : F
	modules : FingerPrint Only
}
{
	mode : KN
	modules : Require both Numberpad & NFC
}
*/

