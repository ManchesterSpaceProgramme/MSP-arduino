#include <SoftwareSerial.h>

SoftwareSerial gps(3,4); // RX, TX  Connect GPS TX to pin 3
void setup()
{
  Serial.begin(9600); // be sure to check lower right corner info in an open 'serial windows'
  gps.begin(9600);  // may be 4800, 19200,38400 or 57600

}

void loop()
{
  if (gps.available())  Serial.write(gps.read());
}
