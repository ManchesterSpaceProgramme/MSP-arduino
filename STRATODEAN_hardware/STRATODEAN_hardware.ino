//Title:    STRATODEAN Tracker code
//Author:   Mark Ireland
//Website:  http://www.stratodean.co.uk
//Notes:    Thanks goes to all the existing UKHAS enthusiasts who helped with providing example code for others to use.
//          Thanks also to the guys on the UKHAS #highaltitude irc channel for their invaluable help.

//Github test
//We need SoftwareSerial port as we are using the hardware port for GPS
//#include <SoftwareSerial.h>
//This is a modifed version of the TinyGPS libaray for UBlox GPS chips
#include <TinyGPS_UBX.h>
//Include our separate code for ease of reading
#include "rtty.h"
#include "gps.h"

//Setup pins
#define NTX2 9
//Character buffer for transmission
#define DATASIZE 256
char data[DATASIZE];
//s_id as sentence id to have a unique number for each transmission
uint16_t s_id = 0;
//Select pin for SD card write
const uint8_t chipSelect = SS;

//initialise our classes
RTTY rtty(NTX2);
GPS gps;
//*************************************************************************************
//
//setup()
//
//*************************************************************************************

void setup() {
  //Intialise our hardware serial port to talk to the GPS at 9600 baud. 
  Serial.begin(9600);
  Serial.println(F("STRATODEAN Payload Tracker, initialising......"));
  
  setPwmFrequency(NTX2, 1);
  
  //Initialise GPS
  gps.start();
}

//*************************************************************************************
//
//loop()
//
//*************************************************************************************

void loop() {
  //$$callsign,sentence_id,time,latitude,longitude,altitude,fix,ascentrate,satellites,batteryvoltage*CHECKSUM\n
  
  //Call gps.get_info and, along with the s_id and battery, put it altogether into the string called 'data'
  snprintf(data, DATASIZE, "$$SDEAN,%d,%s", s_id, gps.get_info());

  //send the data over the radio!
  rtty.send(data);
  //increment the id next time.
  s_id++;
  //delay(500);
}

//*************************************************************************************
//
//helper routines()
//
//*************************************************************************************
//subroutine to give the amount of ram available on the board
int freeRam () {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}

void setPwmFrequency(int pin, int divisor) {
 byte mode;
 if(pin == 5 || pin == 6 || pin == 9 || pin == 10) {
 switch(divisor) {
 case 1:
 mode = 0x01;
 break;
 case 8:
 mode = 0x02;
 break;
 case 64:
 mode = 0x03;
 break;
 case 256:
 mode = 0x04;
 break;
 case 1024:
 mode = 0x05;
 break;
 default:
 return;
 }
 if(pin == 5 || pin == 6) {
 TCCR0B = TCCR0B & 0b11111000 | mode;
 }
 else {
 TCCR1B = TCCR1B & 0b11111000 | mode;
 }
 }
 else if(pin == 3 || pin == 11) {
 switch(divisor) {
 case 1:
 mode = 0x01;
 break;
 case 8:
 mode = 0x02;
 break;
 case 32:
 mode = 0x03;
 break;
 case 64:
 mode = 0x04;
 break;
 case 128:
 mode = 0x05;
 break;
 case 256:
 mode = 0x06;
 break;
 case 1024:
 mode = 0x7;
 break;
 default:
 return;
 }
 TCCR2B = TCCR2B & 0b11111000 | mode;
 }
}
