// Author: TheMiNuS
// License: Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International
//
// In short: You can use this code for personal usage but no commercial use is allowed without contacting me.
//
// All comments converted to English for clarity.

#include <Arduino.h>
#include <stdlib.h>
#include <EEPROM.h>
#include <ArduinoOTA.h>
#include <ESPmDNS.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <Timezone.h>
#include <TimeLib.h>
#include <ESPAsyncWebServer.h>
#include <PubSubClient.h>
#include <HardwareSerial.h>
#include <Update.h>

#include "IntervalTimer.h"
#include "SystemConfiguration.h"
#include "Utils.h"
#include "UacDevice.h"
#include "InternalTimers.h"
#include "webInterfacePages.h"
#include "htmlCode.h"

const char TOPIC_BASE[] PROGMEM = "UnleashedAirConditioner/"; // MQTT base
#define FIRMWARE_VERSION "0.0.1"

#if defined(ESP8266)
// Public signing key for OTA bin signature (BearSSL), put your own key here this one is as an example.
BearSSL::PublicKey signPubKey{R"(-----BEGIN PUBLIC KEY-----
MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAos4s+8A5zGdaiIQJXd3f
2xvQG/ESsNwRMvW8H75/WrACaVocGyemsr5PHRb5UvvsZEov1mHrw3o9b4tahZZR
p1GI90oPyjvoMzWJltmfaNq7QuUihgdQQeUnR2yyiEEhUqfwgsWZ8oEnD7kN7DQQ
B0Z29cc4n5JBGFUSKOWBLtsrmqrHscN2CJyJ5/Sy6mwBU0lOzFhVxZoopcOoqATc
Ej7n8f/leXiXJFRd6FbXDu7Eftm+ZPXrji486CGZHC2512D3F2fKnpDmnTtrjFDI
afLR9r5zs/a2wAntCXzxBI/J/eVOc2V2MUyMKNl77KdLHbVq30m6T5xrL9Tlt60r
ZQIDAQAB
-----END PUBLIC KEY-----)"};
BearSSL::HashSHA256 hash;
BearSSL::SigningVerifier sign{&signPubKey};
#endif

// Objects and variables
// Air conditioner serial link on ESP32 UART2: RX2 = GPIO16, TX2 = GPIO17
HardwareSerial SerialS1(2);
WiFiClient wifiClient;
int status = WL_IDLE_STATUS;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600, 60000);
AsyncWebServer server(80);
size_t content_len;
PubSubClient mqttClient(wifiClient);

// Forward declarations
void Check_System(void);
uint8_t onFunction(uint8_t, uint8_t);
uint8_t offFunction(uint8_t, uint8_t);

// Global singletons from libraries
extern SystemConfiguration config;
UacDevice MyUacDevice(SerialS1, mqttClient, WiFi, TOPIC_BASE, (char)sizeof(TOPIC_BASE));
IntervalTimer timer_check_system(1000, Check_System, nullptr, IntervalTimer::Recurring);
void sendHeartbeat() { MyUacDevice.sendHeartbeatFrame(); }
IntervalTimer heartBeatTX(60000, sendHeartbeat, nullptr, IntervalTimer::Recurring);
InternalTimers MyInternalTimers(mqttClient, WiFi, timeClient, TOPIC_BASE, nullptr, nullptr, 0x300);

// --- Networking helpers kept here to avoid tight coupling ---
void connectWifi() {
  while ((WiFi.status() != WL_CONNECTED) && (WiFi.getMode() != WIFI_AP)) {
    Serial.printf("Connecting to WIFI ");
    if (!strcmp(config.wifi_ssid, "") || !strcmp(config.wifi_password, "")) {
      WiFi.softAP(config.hostname);
      config.flash_led(0, 4);
      Serial.printf(": Recovery Mode !\r\n");
    } else {
      WiFi.begin(config.wifi_ssid, config.wifi_password);
      WiFi.setHostname(config.hostname);
      int count = 0;
      while (WiFi.status() != WL_CONNECTED && count < 10) {
        Serial.printf(".");
        delay(1000);
        count++;
      }
      if (WiFi.status() != WL_CONNECTED && config.WifiConfig == 0xAAAA) {
        Serial.printf("New WIFI config Failure !\r\n");
        config.WifiConfig = 0xAAAA;
        strcpy(config.wifi_ssid, config.old_wifi_ssid);
        strcpy(config.wifi_password, config.old_wifi_password);
        config.save();
        ESP.restart();
      } else if (WiFi.status() == WL_CONNECTED && config.WifiConfig == 0xAAAA) {
        Serial.printf("New WIFI config working !\r\n");
        config.WifiConfig = 0x5555;
        config.save();
        ESP.restart();
      } else {
        Serial.printf(" Connected !\r\n");
      }
    }
  }
}

void connectMqtt() {
  if (strlen(config.mqtt_host) == 0) {
    return;
  }

  if (!mqttClient.connected()) {
    Serial.printf("Connecting to MQTT...\r\n");
    if (mqttClient.connect("ESP8266Client", config.mqtt_login, config.mqtt_password )) {
      Serial.printf("connected\r\n");
    } else {
      Serial.printf("failed with state %d \r\n", mqttClient.state());
    }
  } else {
    mqttClient.loop();
  }
}

void startOTA(){
  // Do not expose OTA if no password is set in EEPROM.
    if (strlen(config.ota_password) == 0) {
  return; // OTA disabled until user sets a password from the web UI
  }
  ArduinoOTA.setPort(8266);
  ArduinoOTA.setHostname(config.hostname);
  ArduinoOTA.setPasswordHash(config.ota_password);
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) type = "sketch";
    else type = "filesystem";
    Serial.printf("Start updating %s", type.c_str());
  });
  ArduinoOTA.onEnd([]() { Serial.printf("\r\nEnd"); });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r\n", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR)      Serial.printf("Auth Failed");
    else if (error == OTA_BEGIN_ERROR)Serial.printf("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.printf("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.printf("Receive Failed");
    else if (error == OTA_END_ERROR)  Serial.printf("End Failed");
   });
  ArduinoOTA.begin();
}

// --- MQTT callback ---
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  uint16_t i=0;
  int16_t configPos = 0;
  String topicStr = String(topic);
  String message = "";
  for (i = 0; i < length; i++) { message = message + (char)payload[i]; }

  if (topicStr.startsWith(String(TOPIC_BASE) + WiFi.macAddress())) {
      configPos = topicStr.indexOf("/AC_config/");
      if (configPos != -1) {
          int variableStartPos = configPos + strlen("/AC_config/");
          String variableName = topicStr.substring(variableStartPos);
          uint8_t macBlocks[6] = { 0, 0, 0, 0, 0, 0 };
          getMacAddressBlocks(WiFi, macBlocks);
          if (MyUacDevice.preFillConfiguration(macBlocks)) {
              MyUacDevice.updateConfiguration(variableName, message.toInt());
              MyUacDevice.sendConfigurationToAC = 1;
          }
      }
      configPos = topicStr.indexOf("/AC_programs_config/");
      if (configPos != -1) {
        int programStartPos = configPos + strlen("/AC_programs_config/");
        int nextSlashPos = topicStr.indexOf('/', programStartPos);
        int programNumber = topicStr.substring(programStartPos, nextSlashPos).toInt();
        String variableName = topicStr.substring(nextSlashPos + 1);
        MyUacDevice.updateProgramsConfiguration(variableName, message.toInt(), programNumber);
      }
      configPos = topicStr.indexOf("/timers_config/");
      if (configPos != -1) {
        int timerStartPos = configPos + strlen("/timers_config/");
        int nextSlashPos = topicStr.indexOf('/', timerStartPos);
        int timerNumber = topicStr.substring(timerStartPos, nextSlashPos).toInt();
        String variableName = topicStr.substring(nextSlashPos + 1);
        MyInternalTimers.updateConfiguration(variableName, message.toInt(), timerNumber);
        MyInternalTimers.publishTimerStates();
      }
  }
}

void setup() {
#if defined(ESP8266)
  Update.installSignature(&hash, &sign);
#endif

  Serial.begin(115200, SERIAL_8N1);
  delay(500);

  config.load();
  connectWifi();

  timeClient.begin();
  startServers();
  startOTA();

  if (strlen(config.mqtt_host) > 0) {
    mqttClient.setServer(config.mqtt_host, config.mqtt_port);
    mqttClient.setCallback(mqttCallback);
    connectMqtt();
  }

  MyUacDevice.begin();
  MyUacDevice.subscribeToTopics();

  SerialS1.begin(9600, SERIAL_8N1, 16, 17);

  MyInternalTimers.begin();
  MyInternalTimers.subscribeToTopics();
  MyInternalTimers.publishTimerStates();
  MyInternalTimers.setOnFunction(onFunction);
  MyInternalTimers.setOffFunction(offFunction);
} 

uint8_t onFunction(uint8_t programID, uint8_t timerID) {
  String topic = String(TOPIC_BASE) + WiFi.macAddress() + "/debug/debug";
  String payload = "Received as argument ProgramID:" + String(programID) + " TimerID:"+ String(timerID) + "  / Turning ON";
  mqttClient.publish(topic.c_str(), payload.c_str());

  topic = String(TOPIC_BASE) + WiFi.macAddress() + "/debug/relay";
  payload = "1";
  mqttClient.publish(topic.c_str(), payload.c_str());
  return 0;
}

uint8_t offFunction(uint8_t programID, uint8_t timerID) {
  String topic = String(TOPIC_BASE) + WiFi.macAddress() + "/debug/debug";
  String payload = "Received as argument ProgramID:" + String(programID) + " TimerID:"+ String(timerID) + "  / Turning OFF";
  mqttClient.publish(topic.c_str(), payload.c_str());

  topic = String(TOPIC_BASE) + WiFi.macAddress() + "/debug/relay";
  payload = "0";
  mqttClient.publish(topic.c_str(), payload.c_str());
  return 0;
}

void Check_System() {
  // System monitoring executed every second.
  connectWifi();
  timeClient.update();
  updateNTPClientOffset(timeClient);
  ArduinoOTA.handle();
  connectMqtt();
  MyInternalTimers.run();
  MyUacDevice.heartBeatControl();
}

void loop() {
  heartBeatTX.run();
  timer_check_system.run();
  MyUacDevice.process();

  static unsigned long lastUartDiag = 0;
  if (millis() - lastUartDiag > 1000) {
    lastUartDiag = millis();
    if (SerialS1.available()) {
      uint8_t b = SerialS1.read();
      Serial.printf("UART2 RX: 0x%02X\n", b);
    }
  }

  yield();
}
