#include <SPI.h>
#include <Ethernet.h>
#include <Modbus.h>
#include <ModbusIP.h>
const int SENSOR_IREG = 100;
const int sensorPin = A0;
ModbusIP mb;
long ts;
void setup() {
  Serial.begin(9600);
  byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
  byte ip[] = { 192, 168, 137, 120 };
  mb.config(mac, ip);
  mb.addIreg(SENSOR_IREG);
  ts = millis();
}

void loop() {
  mb.task();
  if (millis() > ts + 1000) {
    ts = millis();
    mb.Ireg(SENSOR_IREG, analogRead(sensorPin));
  }
}
