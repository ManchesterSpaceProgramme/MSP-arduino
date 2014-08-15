//Title:    STRATODEAN Tracker code
//Author:   Mark Ireland
//Website:  http://www.stratodean.co.uk
//Notes:    Thanks goes to all the existing UKHAS enthusiasts who helped with providing example code for others to use.
//          Thanks also to the guys on the UKHAS #highaltitude irc channel for their invaluable help.

//Github test
//We need SoftwareSerial port as we are using the hardware port for GPS
#include <SoftwareSerial.h>
//This is a modifed version of the TinyGPS libaray for UBlox GPS chips
#include <TinyGPS_UBX.h>
//Include our separate code for ease of reading
#include "rtty.h"
#include "gps.h"

#define GPSTX 11
#define GPSRX 10
#define NTX2 9
//Character buffer for transmission
#define DATASIZE 256
char data[DATASIZE];
//s_id as sentence id to have a unique number for each transmission
uint16_t s_id = 0;
//initialise our classes
RTTY rtty(NTX2);
GPS gps(GPSRX, GPSTX);

//*************************************************************************************
//
//setup()
//
//*************************************************************************************

void setup() {
  //Initialise GPS
  gps.start();
  setPwmFrequency(NTX2, 1);
  Serial.println(F("GPS and SD initialised"));  
}

//*************************************************************************************
//
//loop()
//
//*************************************************************************************

void loop() {
  //Call gps.get_info and, along with the s_id and battery, put it altogether into the string called 'data'
  snprintf(data, DATASIZE, "$$DOGE1,%d,%s", s_id, gps.get_info());
  //print this to the screen and the ram
  //Serial.println(data);
  //Serial.println(freeRam());

  rtty.send(data);
  //Serial.println(freeRam());
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
