//########## Basic Setup & Code ##########

//RFID Code
#include <SPI.h>
#include <RFID.h>
#define SS_PIN 10
#define RST_PIN 9
RFID rfid(SS_PIN, RST_PIN);
String rfidCard;


void setup() {
  //RFID Code
  Serial.begin(9600);
  Serial.println("Starting the RFID Reader...");
  SPI.begin();
  rfid.init();


}

void loop() {
  RFIDRead()
}


//########## Front-end Modules ##########
//------Authentication Modules-----
//RFID Sensor reading
void RFIDRead(){
      if (rfid.isCard()) {
    if (rfid.readCardSerial()) {
      rfidCard = String(rfid.serNum[0]) + " " + String(rfid.serNum[1]) + " " + String(rfid.serNum[2]) + " " + String(rfid.serNum[3]);
      Serial.println(rfidCard);
    }
    rfid.halt();
  }
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
void ReadTouchSens(){
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




