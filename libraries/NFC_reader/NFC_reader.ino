//-------------RFID Module-----------
#include <MFRC522.h>
#include <stdio.h>
#include <SPI.h>

#define RFID_SS_PIN 53  //slave select pin
#define RST_PIN 5       //reset pin

MFRC522 mfrc522(RFID_SS_PIN, RST_PIN);  // instatiate a MFRC522 reader object.
MFRC522::MIFARE_Key key;                //create a MIFARE_Key struct named 'key', which will hold the card information

String rfidCard;
int block = 2;  //this is the block number we will write into and then read. Do not write into 'sector trailer' block, since this can make the block unusable.

byte blockcontent[16] = { "DigitalControl__" };  //an array with 16 bytes to be written into one of the 64 card blocks is defined
//byte blockcontent[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};//all zeros. This can be used to delete a block.
byte readbackblock[18];  //This array is used for reading out a block. The MIFARE_Read method requires a buffer that is at least 18 bytes to hold the 16 bytes of a block.



void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  SPI.begin();         // Init SPI bus
                       //NFC reader module
  mfrc522.PCD_Init();  // Init MFRC522 card (in case you wonder what PCD means: proximity coupling device)
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;  //keyByte is defined in the "MIFARE_Key" 'struct' definition in the .h file of the library
  }
    Serial.println("Scan a MIFARE Classic card");
}

void loop() {
  // put your main code here, to run repeatedly:
  RFIDRead();
}

//RFID Sensor reading
bool RFIDRead() {

  // Look for new cards (in case you wonder what PICC means: proximity integrated circuit card)
  if (!mfrc522.PICC_IsNewCardPresent()) {  //if PICC_IsNewCardPresent returns 1, a new card has been found and we continue
    return false;                                //if it did not find a new card is returns a '0' and we return to the start of the loop
  }
  // Select one of the cards
  if (!mfrc522.PICC_ReadCardSerial()) {  //if PICC_ReadCardSerial returns 1, the "uid" struct (see MFRC522.h lines 238-45)) contains the ID of the read card.
    return false;                              //if it returns a '0' something went wrong and we return to the start of the loop
  }

  Serial.println("card selected");

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
    Serial.print("\nAccess Granted");
    return true;
  } else {
    Serial.print("\nAccess Denied");
    return false;
  }

  //Reset varibles
  flag = false;
  for (int i = 0; i < sizeof(readbackblock); i++) {
    readbackblock[i] = '\0';
  }
  return false;
}


int readBlock(int blockNumber, byte arrayAddress[]) {
  int largestModulo4Number = blockNumber / 4 * 4;
  int trailerBlock = largestModulo4Number + 3;  //determine trailer block for the sector

  /*****************************************authentication of the desired block for access***********************************************************/
  byte status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));

  if (status != MFRC522::STATUS_OK) {
    Serial.print("PCD_Authenticate() failed (read): ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return false;  //return "3" as error message
  }

  /*****************************************reading a block***********************************************************/

  byte buffersize = 18;                                                  //we need to define a variable with the read buffer size, since the MIFARE_Read method below needs a pointer to the variable that contains the size...
  status = mfrc522.MIFARE_Read(blockNumber, arrayAddress, &buffersize);  //&buffersize is a pointer to the buffersize variable; MIFARE_Read requires a pointer instead of just a number
  if (status != MFRC522::STATUS_OK) {
    Serial.print("MIFARE_read() failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return false;  //return "4" as error message
  }
  Serial.println("block was read");
}
