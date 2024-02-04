#ifndef RELAY_H
#define RELAY_H

#include <Arduino.h>

class Relay {
private:
  byte _pin;
  bool _isOn = false;
  void write();

public:
  void begin(const byte pin);
  bool isOn() const;
  bool isOff() const;
  void turnOn();
  void turnOff();
  void toggle();
};

#endif // RELAY_H