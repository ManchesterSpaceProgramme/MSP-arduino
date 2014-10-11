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

#include <SFE_BMP180.h>
#include <Wire.h>


SFE_BMP180 pressure;

#define ALTITUDE 0.0
#define INITIAL_PRESSURE 0.0

//Include our separate code for ease of reading
#include "rtty.h"
#include "gps.h"

#define ENABLE_RADIO 8
#define GPSTX 10
#define GPSRX 11
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
  Serial.begin(9600);
  //Initialise GPS
  gps.start();
  
  //Initialise radio
  setPwmFrequency(NTX2, 1);
  pinMode(ENABLE_RADIO, OUTPUT);
  digitalWrite(ENABLE_RADIO, HIGH);
  Serial.println(F("GPS and Radio initialised"));  
  
  //Initialise BMP
  
  if (pressure.begin())
    Serial.println("BMP180 init success");
  else
  { Serial.println("BMP180 init fail\n\n");
    while(1); // Pause forever.
  }
}

//*************************************************************************************
//
//loop()
//
//*************************************************************************************

void loop() {
  //Call gps.get_info and, along with the s_id, put it altogether into the string called 'data'
  snprintf(data, DATASIZE, "$$DOGE1,%d,%s,%s", s_id, gps.get_info(), getBMP());
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
 
char *getBMP(){
   char status;
   double T,P,p0,a;
   static char bmpReturn[13];
   char temp[4];
   char absPressure[4];
   char bmpAltitude[5];
   
   status = pressure.startTemperature();
  if (status != 0)
  {
    // Wait for the measurement to complete:
    delay(status);
    status = pressure.getTemperature(T);
    if (status != 0)
    {
      //convert float to string
      dtostrf(T, 0, 0, bmpReturn);
      
      status = pressure.startPressure(3);
      if (status != 0)
      {
        // Wait for the measurement to complete:
        delay(status);
        status = pressure.getPressure(P,T);
        if (status != 0)
        {
          strncat(bmpReturn, ",", 1);
          // Print out the measurement:
          dtostrf(P, 0, 0, absPressure);
          strncat(bmpReturn, absPressure, 4);  
          //Serial.print("absolute pressure: ");
          //Serial.print(P,2);
          
          p0 = pressure.sealevel(P,ALTITUDE);
          //Serial.print("relative (sea-level) pressure: ");
          //Serial.print(p0,2);
          
          a = pressure.altitude(P,1011);
                    
          strncat(bmpReturn, ",", 1);
          dtostrf(a, 0, 0, bmpAltitude);
          strncat(bmpReturn, bmpAltitude, 5); 
          //Serial.print("computed altitude: ");
          //Serial.print(a,0);
          //Serial.println(" meters, ");
          
        }
        
      }
  }
 }
 Serial.println(bmpReturn);
      return bmpReturn;
 }
