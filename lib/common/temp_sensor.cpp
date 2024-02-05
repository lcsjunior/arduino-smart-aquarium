#include "temp_sensor.h"

unsigned long requestTemperaturesTime = 0;

void DSTempSensor::begin(const byte pin) {
  _sensors = DallasTemperature(new OneWire(pin));
  _sensors.begin();
}

void DSTempSensor::requestTemperatures() {
  if ((millis() - requestTemperaturesTime) >= REQUEST_TEMPERATURES_TIMEOUT) {
    _sensors.requestTemperatures();
    requestTemperaturesTime = millis();
  }
}

float DSTempSensor::getCTemp() {
  float cTemp = _sensors.getTempCByIndex(0);
  if (cTemp == DEVICE_DISCONNECTED_C) {
    Serial.println(F("Error: Could not read temperature data"));
  }
  return cTemp;
}
