/*
 
  Transmits data via NTX2
  
   
*/
#include <string.h>
#include <util/crc16.h>
int RADIO_MARK_PIN=9;
char DATASTRING[200];
#include <SoftwareSerial.h>
SoftwareSerial GPS(10, 11);
byte gps_set_sucess = 0 ;
#include <TinyGPS.h>
TinyGPS gps;

int count = 1;
byte navmode = 99;
float flat, flon;
unsigned long date, time, chars, fix_age;

int hour = 0 , minute = 0 , second = 0, navstatus = 0; 
int numbersats = 99;
char latbuf[12] = "0", lonbuf[12] = "0", altbuf[12] = "0";
long int ialt = 123;

 
void setup()
{
 pinMode(RADIO_MARK_PIN,OUTPUT);
 setPwmFrequency(RADIO_MARK_PIN, 1);

  GPS.begin(9600); 
  // START OUR SERIAL DEBUG PORT
  Serial.begin(9600);
  Serial.println("GPS Level Convertor Board Test Script");
  Serial.println("gps");
  Serial.println("Initialising....");
  //
  // THE FOLLOWING COMMAND SWITCHES MODULE TO 4800 BAUD
  // THEN SWITCHES THE SOFTWARE SERIAL TO 4,800 BAUD
  //
  GPS.print("$PUBX,41,1,0007,0003,4800,0*13\r\n"); 
  GPS.begin(4800);
  GPS.flush();
 
  //  THIS COMMAND SETS FLIGHT MODE AND CONFIRMS IT 
  
  Serial.println("Setting uBlox nav mode: ");
  uint8_t setNav[] = {
    0xB5, 0x62, 0x06, 0x24, 0x24, 0x00, 0xFF, 0xFF, 0x06, 0x03, 0x00, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00, 0x05, 0x00, 0xFA, 0x00, 0xFA, 0x00, 0x64, 0x00, 0x2C, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16, 0xDC                        };
  while(!gps_set_sucess)
  {
    sendUBX(setNav, sizeof(setNav)/sizeof(uint8_t));
    gps_set_sucess=getUBX_ACK(setNav);
  }
  gps_set_sucess=0;
}
 
  
boolean checkNAV(){
uint8_t b, bytePos = 0;
uint8_t getNAV5[] = { 0xB5, 0x62, 0x06, 0x24, 0x00, 0x00, 0x2A, 0x84 };
Serial.flush();
unsigned long startTime = millis();
sendUBX(getNAV5, sizeof(getNAV5)/sizeof(uint8_t));
  
 while (1) {
    
    if (GPS.available()) {
    Serial.write(GPS.read()); 
    }  
  

 }
}

 
void loop()
{
int year;
  int n;
  byte month, day, hour, minute, second, hundredths;
  unsigned long age;
  Serial.println("In loop");
  
  rtty_txstring(DATASTRING);
  sprintf (DATASTRING, "$$SJohn,%02d:%02d:%02d,%s,%s,%ld\n", hour, minute, second, latbuf, lonbuf, ialt);
  Serial.println(DATASTRING);
 
    Serial.println("GPS availabe and ready to be read");

gps.f_get_position(&flat, &flon, &age);
gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &age);

   gps.get_datetime(&date, &time, &age);
        hour = (time / 1000000);
        minute = ((time - (hour * 1000000)) / 10000);
        second = ((time - ((hour * 1000000) + (minute * 10000))));
        second = second / 100;
      
      //Get Position
      gps.f_get_position(&flat, &flon);
  
      //convert float to string
      dtostrf(flat, 7, 4, latbuf);
      dtostrf(flon, 7, 4, lonbuf);
      
     Serial.println(latbuf);
      //just check that we are putting a space at the front of lonbuf
      if(lonbuf[0] == ' ')
      {
        lonbuf[0] = '+';
      }
      
      // +/- altitude in meters
      ialt = (gps.f_altitude() / 100);   
      itoa(ialt, altbuf, 10);
      Serial.println("Got altitude");

}


 
// Send a byte array of UBX protocol to the GPS
void sendUBX(uint8_t *MSG, uint8_t len) {
  for(int i=0; i<len; i++) {
    GPS.write(MSG[i]);
    Serial.print(MSG[i], HEX);
  }
  GPS.println();
}
 
 
// Calculate expected UBX ACK packet and parse UBX response from GPS
boolean getUBX_ACK(uint8_t *MSG) {
  uint8_t b;
  uint8_t ackByteID = 0;
  uint8_t ackPacket[10];
  unsigned long startTime = millis();
  Serial.print(" * Reading ACK response: ");
 
  // Construct the expected ACK packet    
  ackPacket[0] = 0xB5;	// header
  ackPacket[1] = 0x62;	// header
  ackPacket[2] = 0x05;	// class
  ackPacket[3] = 0x01;	// id
  ackPacket[4] = 0x02;	// length
  ackPacket[5] = 0x00;
  ackPacket[6] = MSG[2];	// ACK class
  ackPacket[7] = MSG[3];	// ACK id
  ackPacket[8] = 0;		// CK_A
  ackPacket[9] = 0;		// CK_B
 
  // Calculate the checksums
  for (uint8_t i=2; i<8; i++) {
    ackPacket[8] = ackPacket[8] + ackPacket[i];
    ackPacket[9] = ackPacket[9] + ackPacket[8];
  }
 
  while (1) {
 
    // Test for success
    if (ackByteID > 9) {
      // All packets in order!
      Serial.println(" (SUCCESS!)");
      return true;
    }
 
    // Timeout if no valid response in 3 seconds
    if (millis() - startTime > 3000) { 
      Serial.println(" (FAILED!)");
      return false;
    }
 
    // Make sure data is available to read
    if (GPS.available()) {
      b = GPS.read();
 
      // Check that bytes arrive in sequence as per expected ACK packet
      if (b == ackPacket[ackByteID]) { 
        ackByteID++;
        Serial.print(b, HEX);
      } 
      else {
        ackByteID = 0;	// Reset and look again, invalid order
      }
 
    }
  }
}

 
void rtty_txstring (char * string)
{
 
    /* Simple function to sent a char at a time to
    ** rtty_txbyte function.
    ** NB Each char is one byte (8 Bits)
    */
 
    char c;
 
    c = *string++;

   
    while ( c != '\0')
    {

        rtty_txbyte (c);
        c = *string++;
    }

}
 
void rtty_txbyte (char c)
{
    /* Simple function to sent each bit of a char to
    ** rtty_txbit function.
    ** NB The bits are sent Least Significant Bit first
    **
    ** All chars should be preceded with a 0 and
    ** proceded with a 1. 0 = Start bit; 1 = Stop bit
    **
    */
 
    int i;
 
    rtty_txbit (0); // Start bit
 
    // Send bits for for char LSB first
 
    for (i=0;i<7;i++) // Change this here 7 or 8 for ASCII-7 / ASCII-8  
   
    {       
    if (c & 1) rtty_txbit(1);       
    else rtty_txbit(0);         
    c = c >> 1;
 
    }
 
    rtty_txbit (1); // Stop bit
}
 
void rtty_txbit (int bit)
{
        if (bit)
        {
          // high
                    digitalWrite(RADIO_MARK_PIN, HIGH);
                    
        }
        else
        {
          // low
                    
                    digitalWrite(RADIO_MARK_PIN, LOW);
 
        }
//                delayMicroseconds(1680); // 600 baud unlikely to work.
//                  delayMicroseconds(3370); // 300 baud
                delayMicroseconds(10000); // For 50 Baud uncomment this and the line below.
                delayMicroseconds(10150); // For some reason you can't do 20150 it just doesn't work.
 
}

uint16_t gps_CRC16_checksum (char *string)
{
    size_t i;
    uint16_t crc;
    uint8_t c;
 
    crc = 0xFFFF;
 
    // Calculate checksum ignoring the first two $s
    for (i = 2; i < strlen(string); i++)
    {
        c = string[i];
        crc = _crc_xmodem_update (crc, c);
    }
 
    return crc;
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
