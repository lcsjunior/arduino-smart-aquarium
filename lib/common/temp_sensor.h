#ifndef TEMP_SENSOR_H
#define TEMP_SENSOR_H

#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define REQUEST_TEMPERATURES_TIMEOUT 300UL

class TempSensor {
public:
  virtual void begin(const byte pin) = 0;
  virtual void requestTemperatures() = 0;
  virtual float getCTemp() = 0;
};

class DSTempSensor : public TempSensor {
private:
  DallasTemperature _sensors;

public:
  void begin(const byte pin);
  void requestTemperatures();
  float getCTemp();
};

#endif // TEMP_SENSOR_H