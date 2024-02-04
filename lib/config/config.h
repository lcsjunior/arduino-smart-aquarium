#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

#define JSON_DOC_SIZE 1024

struct Config {
  float setpoint;
  float hysteresis;
};
extern Config config;

bool loadConfigFile();
void saveConfigFile();
void removeConfigFile();
void printConfigFile();

#endif // CONFIG_H