#include <SPI.h>
#include <Ethernet.h>
#include <Modbus.h>
#include <ModbusIP.h>
#include <SoftwareSerial.h>
SoftwareSerial pmsSerial(2, 3);
const int SENSOR_IREG1 = 4000;
const int SENSOR_IREG2 = 4001;
const int SENSOR_IREG3 = 4002;
ModbusIP mb;
long ts;

void setup() {
  Serial.begin(115200);
  pmsSerial.begin(9600);
  byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
  byte ip[] = { 10, 20, 15, 230 }; //IP Address
  mb.config(mac, ip);
  mb.addIreg(SENSOR_IREG1);
  mb.addIreg(SENSOR_IREG2);
  mb.addIreg(SENSOR_IREG3);
  ts = millis();
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

void loop() {
  mb.task();
  if (millis() > ts + 1000) {
    ts = millis();
    mb.Ireg(SENSOR_IREG1, data.pm10_env);
    mb.Ireg(SENSOR_IREG2, data.pm25_env);
    mb.Ireg(SENSOR_IREG3, data.pm100_env);
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
