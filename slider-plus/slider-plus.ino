//########## Basic Setup & Code ##########
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

byte pin_rows[ROW_NUM] = {5, 4, 3, 2}; //connect to the row pinouts of the keypad
byte pin_column[COLUMN_NUM] = {9, 8, 7, 6}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM );

const int passwordLength = 4;  // Define the length of the password
char password[passwordLength] = {'1', '2', '3', '4'};  // Define the correct password
char enteredPassword[passwordLength];  // Variable to store the entered password
int keyIndex = 0;  // Index to keep track of the key being entered



//########## Setup & Code ##########
void setup() {
  Serial.begin(9600);

  //Touch Sensor Module
  pinMode(TouchSensor_Pin, INPUT);
}

void loop() {
  NumPadRead();
  Serial.println(ReadTouchSens());
}


//########## Front-end Modules ##########
//------Authentication Modules-----
//RFID Sensor reading
void RFIDRead(){
}
//NumberPad reading
void NumPadRead(){
  char key = keypad.getKey();
    // Ignore any non-digit characters
    if (isdigit(key)) {
      enteredPassword[keyIndex] = key;
      keyIndex++;
      Serial.print(key);

      // Check if all digits have been entered
      if (keyIndex == passwordLength) {
        bool correctPassword = true;
        for (int i = 0; i < passwordLength; i++) {
          if (enteredPassword[i] != password[i]) {
            correctPassword = false;
            break;
          }
        }

        if (correctPassword) {
          Serial.println("Correct password entered.");
        } else {
          Serial.println("Wrong password entered.");
        }
        // Reset variables for the next password entry
        keyIndex = 0;
        memset(enteredPassword, 0, sizeof(enteredPassword));
      }
    }
}
//Fingerprint reading
void FingerprintRead(){
}

//--------Other Modules-------
//Closing the door when someone leaves the room using motion sensor
int ReadMotionSens(){
}
//Buzzer write when opening and closing the door
void WriteToBuzzer(){
}


//########## Back-end Modules ##########
//Opening Door from the inside using touch sensor
bool ReadTouchSens(){
  bool TouchState = digitalRead(TouchSensor_Pin);
  return TouchState;
  delay(10);
}
//Reading configurations from the SD Card
void ReadSDCard(){
}
//Reading the Door sensor to verify door status
void ReadDoorSens(){
}
//Controlling the door using the motor
void ControlDoor(){
}




