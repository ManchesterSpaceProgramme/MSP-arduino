//class definition for the GPS functions
#ifndef GPS_H
#define GPS_H

#include <Arduino.h>

#define BUFSIZE 128

//class definition

class GPS: 
public SoftwareSerial {
public:
  GPS(uint8_t rx_pin, uint8_t tx_pin);
  void start();
  char *get_info();
  boolean hasFix();
private:
  void send_ubx(uint8_t *msg, uint8_t len);
  boolean get_ubx_ack(uint8_t *msg);
  boolean poll();
};

#endif


