#include <SPI.h>
#include <Ethernet.h>
#include "Mudbus.h"
#include <SoftwareSerial.h>
SoftwareSerial pmsSerial(2, 3);
Mudbus Mb;

void setup()
{
  uint8_t mac[]     = { 0x90, 0xA2, 0xDA, 0x00, 0x51, 0x06 };
  uint8_t ip[]      = { 192, 168, 137, 120 };
  uint8_t gateway[] = { 192, 168, 137, 1 };
  uint8_t subnet[]  = { 255, 255, 255, 0 };
  Ethernet.begin(mac, ip, gateway, subnet);
  delay(5000);
  Serial.begin(115200);
  pmsSerial.begin(9600);
}
struct pms5003data {
  uint16_t framelen;
  uint16_t pm10_standard, pm25_standard, pm100_standard;
  uint16_t pm10_env, pm25_env, pm100_env;
  uint16_t particles_03um, particles_05um, particles_10um, particles_25um, particles_50um, particles_100um;
  uint16_t unused;
  uint16_t checksum;
};
struct pms5003data data;
void loop()
{
  if (readPMSdata(&pmsSerial)) {
    Mb.Run();
    Mb.R[0] = data.pm10_env;
    Mb.R[1] = data.pm25_env;
    Mb.R[2] = data.pm100_env;
  }
}
boolean readPMSdata(Stream *s) {
  if (! s->available()) {
    return false;
  }
  if (s->peek() != 0x42) {
    s->read();
    return false;
  }
  if (s->available() < 32) {
    return false;
  }
  uint8_t buffer[32];
  uint16_t sum = 0;
  s->readBytes(buffer, 32);
  for (uint8_t i = 0; i < 30; i++) {
    sum += buffer[i];
  }
  uint16_t buffer_u16[15];
  for (uint8_t i = 0; i < 15; i++) {
    buffer_u16[i] = buffer[2 + i * 2 + 1];
    buffer_u16[i] += (buffer[2 + i * 2] << 8);
  }
  memcpy((void *)&data, (void *)buffer_u16, 30);
  if (sum != data.checksum) {
    Serial.println("Checksum failure");
    return false;
  }
  return true;
}
