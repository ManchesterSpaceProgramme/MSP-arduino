//-----------------------------------------------------------------------------------------------------------------------
// Manchester Space Program Mission 3 [KAREN] Payload firmware
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

void setup()
{
  unsigned long GPSTimer;
  
  pinMode(CAMERA, OUTPUT);
  pinMode(VOLTAGE, INPUT);
  pinMode(ENABLE_RADIO, OUTPUT);
  
  Serial.begin(9600);
  Serial.println("KAREN Startup sequence. . .");
  
  Serial.println("BMP085 init start");
  if (pressure.begin()) {
    //Read from BMP to get initial pressure readings
    BMPread();
    Serial.println("BMP085 init success");
  } else {
    Serial.println("BMP085 init fail\n\n");
    
    // start radio and broadcast error message for ever
    setPwmFrequency(NTX2, 1);
    digitalWrite(ENABLE_RADIO, HIGH);
  
    while(1) {
      rtty.send("BMP085 failed to initialize. . .");
    }
  }
  
  // Initialise GPS and wait for lock
  Serial.println("GPS init start.");
  gps.start();
  Serial.println(F("GPS init complete."));
  GPSTimer = millis();
  while(!gps.hasFix())
  {
    Serial.print("Waiting for GPS lock; [");
    Serial.print(gps.get_info());
    Serial.println("]");  
  }
  GPSTimer = (millis() - GPSTimer) / 1000;
  Serial.print("GPS Lock took : ");
  if (GPSTimer > 60){
    Serial.print(GPSTimer/60);
    Serial.print(" mins and ");
  }
  Serial.print(GPSTimer%60);
  Serial.println(" seconds");
  
  // Turn camera on
  digitalWrite(CAMERA, HIGH);
  CameraOn = true;
  CamCyclesBelowVoltage = 0;
  Serial.println("Camera on.");
  
  // Turn Radio on
  setPwmFrequency(NTX2, 1);
  digitalWrite(ENABLE_RADIO, HIGH);
  Serial.println("Radio on");
  
  Serial.println("KAREN Startup complete. . .");
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
    Serial.println("Camera off due to low battery");
  }
  
  // get readings for transmission
  BMPread();
  int T = BMP_Temp*10.0;
  int P = BMP_Pressure*10.0;
  //char Cam = '0';
  //if (CameraOn) Cam = '1';
  
  // format data string to broadcast
  // Callsign, Time, Lat, Long, GPS Alt, Satellites, Battery, Temp, Pressure
  snprintf(data, DATASIZE, "$$KAREN,%d,%s,%d.%d,%d.%d,%d.%d",  s_id,
                                                                      gps.get_info(),
                                                                      V/100, V%100,
                                                                      T/10, abs(T%10),
                                                                      P/10, P%10);
  
  // broadcast the string
  rtty.send(data);
  s_id++;  //increment the id
  delay(1000);
  
  
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

