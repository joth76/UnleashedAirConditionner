#pragma once
#include <Arduino.h>
#include <EEPROM.h>
#include <PubSubClient.h>
#include <NTPClient.h>
#include <WiFi.h>
using ESP8266WiFiClass = WiFiClass;

class InternalTimers {
  public:
    typedef uint8_t (*onFunction)(uint8_t programId, uint8_t timerId);
    typedef uint8_t (*offFunction)(uint8_t programId, uint8_t timerId);

    InternalTimers(PubSubClient& pubSubClient, ESP8266WiFiClass& wifiClient, NTPClient& timeClient,
                   const char* topicPtr, onFunction setOnFuncPtr = nullptr, offFunction setOffFuncPtr = nullptr,
                   uint16_t eepromTimerStartAddressBase = 0x300 );

    ~InternalTimers();

    void setOnFunction(onFunction func);
    void setOffFunction(offFunction func);

    void begin();
    void subscribeToTopics();
    void publishTimerStates();
    void updateConfiguration(String targetParameter = "", uint64_t value = 0, uint16_t bank = 0);
    void run();

  private:
    enum timersParameters { NUM_TIMERS = 16 };

    PubSubClient &mqtt;
    ESP8266WiFiClass &wifi;
    NTPClient &ntpClient;
    const char* topic_base;
    onFunction switchOn;
    offFunction switchOff;
    uint16_t eepromTimerStartAddress;
    bool isCallbackExecuted[timersParameters::NUM_TIMERS];

    struct eepromTimers {
      uint64_t bank[timersParameters::NUM_TIMERS];
      uint8_t areTimersConfigured;
    };
    eepromTimers MyEEPROMTimers;
};
