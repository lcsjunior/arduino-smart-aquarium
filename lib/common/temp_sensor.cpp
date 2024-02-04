#include "temp_sensor.h"

void DSTempSensor::begin(const byte pin) {
  _sensors = DallasTemperature(new OneWire(pin));
  _sensors.begin();
}

void DSTempSensor::requestTemperatures() {
  Serial.print(F("Requesting temperatures..."));
  _sensors.requestTemperatures();
  Serial.println(F("DONE"));
}

float DSTempSensor::getCTemp() {
  float cTemp = _sensors.getTempCByIndex(0);
  if (cTemp == DEVICE_DISCONNECTED_C) {
    Serial.println(F("Error: Could not read temperature data"));
  }
  Serial.print(F("Temperature for the device (index 0) is: "));
  Serial.println(cTemp);
  return cTemp;
}
