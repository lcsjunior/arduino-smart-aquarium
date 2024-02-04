#include <Arduino.h>
#include <PubSubClient.h>
#include <CronAlarms.h>
#include <NoDelay.h>
#include <WebServer.h>
#include <secrets.h>
#include <config.h>
#include <espx_wifi.h>
#include <relay.h>
#include <temp_sensor.h>
#include <thermostat.h>

#define RELAY_PIN_1 21
#define RELAY_PIN_2 19
#define RELAY_PIN_3 18
#define RELAY_PIN_4 5

#define DS_PIN_1 22
#define DS_PIN_2 23

#define MQTT_CONN_TIMEOUT (MILLIS_PER_SECOND * 5)
#define MQTT_PUB_TIMEOUT (MILLIS_PER_SECOND * 30)

const char *mqttServer = "mqtt3.thingspeak.com";
const int mqttPort = 1883;
const char *mqttClientId = MQTT_CLIENT_ID;
const char *mqttUsername = MQTT_USERNAME;
const char *mqttPasswd = MQTT_PASSWORD;
unsigned long channelId = SMART_AQUARIUM_CH_ID;

const char *cronstr_at_07_30 = "0 30 7 * * *";
const char *cronstr_at_08_00 = "0 0 8 * * *";
const char *cronstr_at_14_30 = "0 30 14 * * *";
const char *cronstr_at_15_00 = "0 0 15 * * *";

WiFiClient client;
PubSubClient mqttClient(mqttServer, mqttPort, client);
WebServer server(80);

noDelay mqttConnTime(MQTT_CONN_TIMEOUT);
noDelay mqttPubTime(MQTT_PUB_TIMEOUT);

char topic[32];
char msg[255];

DSTempSensor tempSensor1;
DSTempSensor tempSensor2;
Relay heater1;
Relay heater2;
Relay lamp;
Relay co2Valve;

Thermostat thermostat1(&heater1);
Thermostat thermostat2(&heater2);

void writeMsg();
void mqttConn();
void mqttPub();
void mqttSub();
void initWS();

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);

  tempSensor1.begin(DS_PIN_1);
  tempSensor2.begin(DS_PIN_2);
  heater1.begin(RELAY_PIN_1);
  heater2.begin(RELAY_PIN_3);
  lamp.begin(RELAY_PIN_2);
  co2Valve.begin(RELAY_PIN_4);

  mountFS();
  if (!loadConfigFile()) {
    Serial.println(F("Using default config"));
    config.setpoint = 24;
    config.hysteresis = 0.5;
    saveConfigFile();
  }

  thermostat1.begin(config.setpoint, config.hysteresis, 0, 30);
  thermostat2.begin(config.setpoint, config.hysteresis, 0, 30);

  WiFi.mode(WIFI_AP_STA);
  Wifi.initAP();
  Wifi.initSTA();

  mqttConn();
  initWS();

  Cron.create((char *)cronstr_at_07_30, []() { co2Valve.turnOn(); }, false);
  Cron.create((char *)cronstr_at_08_00, []() { lamp.turnOn(); }, false);
  Cron.create((char *)cronstr_at_14_30, []() { co2Valve.turnOff(); }, false);
  Cron.create((char *)cronstr_at_15_00, []() { lamp.turnOff(); }, false);
}

void loop() {
  Wifi.loop();
  Cron.delay();

  tempSensor1.requestTemperatures();
  tempSensor2.requestTemperatures();

  thermostat1.handleHeater(tempSensor1.getCTemp());
  thermostat2.handleHeater(tempSensor2.getCTemp());

  if (!mqttClient.connected() && mqttConnTime.update()) {
    Serial.println(F("Reconnecting to MQTT..."));
    mqttConn();
  };
  mqttClient.loop();

  if (mqttPubTime.update()) {
    writeMsg();
    mqttPub();
  }

  server.handleClient();
  delay(300);
}

void writeMsg() {
  char tbuf[64];
  getLocalTimeFmt(tbuf, sizeof(tbuf));
  snprintf_P(
      msg, sizeof(msg),
      PSTR("field1=%.1f&field2=%.1f&field3=%d&field4=%d&field5=%d&field6=%d&"
           "status=PUB %s RSSI %d dBm (%d pcent)"),
      tempSensor1.getCTemp(), tempSensor2.getCTemp(), heater1.isOn(),
      heater1.isOn(), lamp.isOn(), co2Valve.isOn(), tbuf, WiFi.RSSI(),
      dBm2Quality(WiFi.RSSI()));
}

void mqttConn() {
  mqttClient.connect(mqttClientId, mqttUsername, mqttPasswd);
  mqttSub();
}

void mqttPub() {
  snprintf_P(topic, sizeof(topic), PSTR("channels/%ld/publish"), channelId);
  Serial.print(topic);
  Serial.print(F(" - "));
  Serial.print(msg);
  Serial.print(F(" ("));
  Serial.print(strnlen_P(msg, sizeof(msg)));
  Serial.println(F(" bytes)"));
  mqttClient.publish(topic, msg);
}

void mqttSub() {
  snprintf_P(topic, sizeof(topic), PSTR("channels/%ld/subscribe"), channelId);
  mqttClient.subscribe(topic);
}

void initWS() {
  server.on(F("/"), []() {
    server.send(200, FPSTR(TEXT_PLAIN), FPSTR("Hello from ESP!"));
  });

  server.on(F("/reboot"), []() {
    Wifi.reboot();
    server.send(200);
  });

  server.on(F("/msg"), []() {
    char buf[sizeof(msg) + 32];
    size_t len = strnlen_P(msg, sizeof(msg));
    snprintf_P(buf, sizeof(buf), PSTR("%s (%zd bytes)"), msg, len);
    server.send(200, FPSTR(TEXT_PLAIN), FPSTR(buf));
  });

  server.on(F("/lamp/toggle"), []() {
    lamp.toggle();
    server.send(200);
  });

  server.onNotFound(
      []() { server.send(404, FPSTR(TEXT_PLAIN), FPSTR("File not found")); });

  server.begin();
  Serial.println(F("HTTP server started"));
}