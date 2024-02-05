#include "temp_sensor.h"

void DSTempSensor::begin(const byte pin) {
  _sensors = DallasTemperature(new OneWire(pin));
  _sensors.begin();
}

void DSTempSensor::requestTemperatures() {
  if ((millis() - _requestTemperaturesTime) >= REQUEST_TEMPERATURES_TIMEOUT) {
    _sensors.requestTemperatures();
    _requestTemperaturesTime = millis();
  }
}

float DSTempSensor::getCTemp() {
  float cTemp = _sensors.getTempCByIndex(0);
  if (cTemp == DEVICE_DISCONNECTED_C) {
    Serial.println(F("Error: Could not read temperature data"));
  }
  return cTemp;
}
