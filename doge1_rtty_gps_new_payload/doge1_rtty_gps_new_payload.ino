//-----------------------------------------------------------------------------------------------------------------------
// Manchester Space Program Mission 1 [DOGE1] Payload firmware
// Authors:   Harvinder Atwal, Pete Blacker
// Website:   http://www.manchesterspaceprogramme.org/
// Notes:     Thanks goes to all the existing UKHAS enthusiasts who helped with providing example code for others to use.
//            Thanks also to the guys on the UKHAS #highaltitude irc channel for their invaluable help.
//-----------------------------------------------------------------------------------------------------------------------

#include <SoftwareSerial.h>
#include <TinyGPS_UBX.h>
#include <SFE_BMP180.h>
#include <Wire.h>

#include "rtty.h"
#include "gps.h"

// set pin assignments 
#define ENABLE_RADIO 8
#define GPSTX 4
#define GPSRX 3
#define NTX2 9
#define CAMERA 2
#define VOLTAGE 17

//initialise our classes
RTTY rtty(NTX2);
GPS gps(GPSRX, GPSTX);
SFE_BMP180 pressure;

// ---- Global Variables ----

// set camera cutoff voltage in 100ths of a volt
#define CAMERA_CUTOFF_VOLTAGE 510
#define CAMERA_CYCLES_B4_CUTOFF 10

unsigned int CamCyclesBelowVoltage;
boolean CameraOn;

//Character buffer for transmission
#define DATASIZE 256
char data[DATASIZE];

//s_id as sentence id to have a unique number for each transmission
uint16_t s_id = 0;

double BMP_Temp, BMP_Pressure;

//*************************************************************************************
//
//setup()
//
//*************************************************************************************

void setup() {
  Serial.begin(9600);
  Serial.println("DOGE2 Startup sequence. . .");
  pinMode(CAMERA, OUTPUT);
  pinMode(VOLTAGE, INPUT);
  setPwmFrequency(NTX2, 1);
  pinMode(ENABLE_RADIO, OUTPUT);
  digitalWrite(ENABLE_RADIO, HIGH);
  Serial.println("Radio init complete");
  Serial.println("BMP085 init start");
  if (pressure.begin())
    Serial.println("BMP085 init success");
  else
  {
    Serial.println("BMP085 init fail\n\n");
    while(1); // Pause forever.
  }
  //Initialise GPS
  Serial.println("GPS init start.");
  gps.start();
  Serial.println(F("GPS init complete."));
  
  // Turn camera on
  digitalWrite(CAMERA, HIGH);
  CameraOn = true;
  CamCyclesBelowVoltage = 0;
  Serial.println("Camera on.");
  Serial.println("DOGE2 Startup complete. . .");
}

//*************************************************************************************
//
//loop()
//
//*************************************************************************************

void loop() {
  
  int V=getVoltage();
  
  // check if the batteries are below the camera cutoff voltage
  if (V < CAMERA_CUTOFF_VOLTAGE) ++CamCyclesBelowVoltage;
  else CamCyclesBelowVoltage=0;
  if (CamCyclesBelowVoltage > CAMERA_CYCLES_B4_CUTOFF) {
    digitalWrite(CAMERA, LOW);
    CameraOn = false;
  }
  
  // get readings for transmission
  BMPread();
  int T = BMP_Temp*10.0;
  int P = BMP_Pressure*10.0;
  int Alt = pressure.altitude(BMP_Pressure,1011)*10;
  char Cam = '0';
  if (CameraOn) Cam = '1';
  
  // format data string to broadcast
  snprintf(data, DATASIZE, "$$DOGE2,%d,GPS=[%s],V=%d.%d,T=%d.%d,P=%d.%d,Alt=%d.%d,Cam=%c",  s_id,
                                                                      gps.get_info(),
                                                                      V/100, V%100,
                                                                      T/10, T%10,
                                                                      P/10, P%10,
                                                                      Alt/10, Alt%10,
                                                                      Cam);
  
  // broadcast the string
  rtty.send(data);
  s_id++;  //increment the id
  
  Serial.println(data);
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

int getVoltage()
{
  float fvoltage;
  int ivoltage;
  
  // Obtain RAW voltage data
  ivoltage = analogRead(VOLTAGE);
  
  // calculate actual battery
  fvoltage = (ivoltage*10.0)/1024.0;
  
  // convert to 100ths of a volt
  ivoltage = fvoltage*100;
  
  return (ivoltage);
}

void BMPread()
{
  char status;
  double T,P;
 
  // read temperature
  status = pressure.startTemperature();
  delay(status);
  status = pressure.getTemperature(T);
  
  // read pressure
  status = pressure.startPressure(3);
  delay(status);
  status = pressure.getPressure(P,T);
 
  // set global variables so we can 'return' 2 values at once
  BMP_Temp = T;
  BMP_Pressure = P;
}

