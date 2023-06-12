//########## Basic Setup & Code ##########
//-----------Common for all Module---------
int currentDoorState; //current status of the door
int lastDoorState; //last status of the door
char securityMode = 'A'; //select the mode of Multifactor Authentication
int timeOut = 0; //Motion sensor timeout

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
Keypad keypad = Keypad( makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM);

const int passLength = 4;
char password[passLength] = "1234";  // Define the correct password
char enteredPassword[passLength];  // Variable to store the entered password
int keyIndex = 0;  // Index to keep track of the key being entered

//-----------Buzzer Module -----------
const int buzzerPin = 9;    // Pin connected to the buzzer
int volume = 255;

//-----------SD Module----------------
#include <SPI.h>
#include <SD.h>
File myFile;
const int chipSelect = 10;
const int misoPin = 50;
const int mosiPin = 51;
const int sckPin = 52;
const int csPin = 53;

//-----------Ultrasonic Sensor------------
const int trigPin = 3;    //Trigger pin of the ultrasonic sensor
const int echoPin = 2;    //Echo pin of the ultrasonic sensor
const int thresholdDistance = 100;  // Threshold distance for triggering the door closing in centimeters
int MotionStatus = 0;

//------------Door Sensor------------
const int DOOR_SENSOR_PIN = 13;

//-------------DC Motor----------------
const int motorPwm = 2;    // initializing pin 2 as pwm
const int motorIn_1 = 8;
const int motorIn_2 = 9;



//########## Setup & Code ##########
void setup() {
  Serial.begin(9600);

  //Buzzer Module
  pinMode(buzzerPin, OUTPUT);

  //Touch Sensor Module
  pinMode(TouchSensor_Pin, INPUT);

  //Motion Sensor Module
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  //SD module
  if (!SD.begin()) {
    Serial.println("initialization failed!");
  }
  
  //Door Sensor Module
  pinMode(DOOR_SENSOR_PIN, INPUT_PULLUP); 
  currentDoorState = digitalRead(DOOR_SENSOR_PIN); 

  //DC Motor
  pinMode(motorPwm, OUTPUT);   // we have to set PWM pin as output
  pinMode(motorIn_1, OUTPUT);  // Logic pins are also set as output
  pinMode(motorIn_2, OUTPUT);

  //reading from the file and setup password and mode
  Serial.println(ReadSDCard());
}

void loop() {
  // Choosing security methods depend on the mode
  if(currentDoorState == 0){
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
  //printing current door status
  Serial.print("Door Status : ");
  Serial.println(ReadDoorSens());

  // Configuring how to function the inside touch button
  if(ReadTouchSens() && currentDoorState == 1){
    closeDoor();
  } else if(ReadTouchSens() && currentDoorState == 0){
    openDoor();
  }
  delay(50);

  //automatically closing the door after few time
  if(currentDoorState == 1 && ReadMotionSens() == 0){
    Serial.println(timeOut++);
    if(timeOut > 100 && MotionStatus == 0){
      closeDoor();
      MotionStatus = 1;
    }
  }
}

//########## Functions ##########
void openDoor(){
  Serial.println("Door is opening...");
  WriteToBuzzer(1);
}
void closeDoor(){
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
          WriteToBuzzer(0);
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
void WriteToBuzzer(bool inputValue){
  if (inputValue == HIGH) {
        // Number 1 is entered, sound the buzzer
        analogWrite(buzzerPin, volume);
        Serial.println("Door is moving...");
        delay(1000);  // Buzz for 1 second
        analogWrite(buzzerPin, 0);
      }
    if (inputValue == LOW){
        analogWrite(buzzerPin, volume-20);
        delay(100);
        analogWrite(buzzerPin, 0);
        delay(100);
        analogWrite(buzzerPin, volume-20);
        delay(100);
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
  File file = SD.open(filename);
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
    String config[5];
    for (i = 0; i<line.length(); i++){
      char c = line.charAt(i);
      if (c == ','){
        config[i] = line.charAt(i-1);
        config[i+1] = line.substring(i+1);
      }
    }
    Serial.println(config[0]);
    Serial.println(config[1]);  
    file.close();
    return config;
  } else {
    Serial.println("Error opening file.");
    return;
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
bool ReadDoorSens(){
  lastDoorState = currentDoorState;
  currentDoorState  = digitalRead(DOOR_SENSOR_PIN);
  delay(50);
  if (lastDoorState == LOW && currentDoorState == HIGH) {
    Serial.println("The door-opening event is detected");
    return 1;
  }
  else
    delay(50);
  if (lastDoorState == HIGH && currentDoorState == LOW) {
    Serial.println("The door-closing event is detected");
    timeOut = 0;
    return 0;
  }
}

//Controlling the door using the motor
void ControlDoor(int direction){
    if (direction == HIGH) {
      digitalWrite(motorIn_1, HIGH);
      digitalWrite(motorIn_2, LOW);
      analogWrite(motorPwm, 255);
      // Clockwise for 3 secs
      delay(3000);
      }
    else if (direction == LOW){
      digitalWrite(motorIn_1, LOW);
      digitalWrite(motorIn_2, HIGH);
      delay(3000);
    }
    digitalWrite(motorIn_1, HIGH);
    digitalWrite(motorIn_2, HIGH);
    delay(1000);
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

