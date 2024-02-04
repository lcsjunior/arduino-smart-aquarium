#include "thermostat.h"

unsigned long stateExitTime = 0;

void Thermostat::begin(const float setpoint, const float hysteresis,
                       const float lowerLimit, const float upperLimit) {
  _setpoint = setpoint;
  _hysteresis = hysteresis;
  _lowerLimit = lowerLimit;
  _upperLimit = upperLimit;
}

ThermostatState Thermostat::getState() const { return _state; }

void Thermostat::setState(ThermostatState state) {
  stateExitTime = millis();
  _state = state;
}

char *Thermostat::getStatus() const {
  switch (_state) {
  case IDLE:
    return (char *)"Idle";
  case COOLING:
    return (char *)"C";
  case HEATING:
    return (char *)"H";
  default:
    return (char *)"Undef";
  }
}

void Thermostat::handleCooler(float cTemp) {}

void Thermostat::handleHeater(float cTemp) {
  if (isnan(cTemp) || cTemp < _lowerLimit || cTemp > _upperLimit) {
    setState(IDLE);
  }

  switch (_state) {
  case IDLE:
    _k->turnOff();
    if ((millis() - stateExitTime) >= IDLE_TIMEOUT) {
      setState(COOLING);
    }
    break;
  case COOLING:
    _k->turnOff();
    if (cTemp < _setpoint) {
      setState(HEATING);
    }
    break;
  case HEATING:
    _k->turnOn();
    if (cTemp > _setpoint + _hysteresis) {
      setState(IDLE);
    }
  }
}
