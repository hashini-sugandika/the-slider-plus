//########## Basic Setup & Code ##########
//Touch Sensor Module
const int TouchSensor_Pin = A0;


void setup() {
  Serial.begin(9600);

  //Touch Sensor Module
  pinMode(TouchSensor_Pin, INPUT);
}

void loop() {


  Serial.println(ReadTouchSens());
}


//########## Front-end Modules ##########
//------Authentication Modules-----
//RFID Sensor reading
void RFIDRead(){
}
//NumberPad reading
void NumPadRead(){

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




