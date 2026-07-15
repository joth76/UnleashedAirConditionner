#pragma once
#include <Arduino.h>
#include <EEPROM.h>
#include <PubSubClient.h>
#include <WiFi.h>
using ESP8266WiFiClass = WiFiClass;
#include "Utils.h"

class UacDevice {
  public:
    bool isConfigurationUpToDate;        // Whether we decoded a valid configuration (CRC OK)
    bool sendConfigurationToAC;

    uint16_t returnAirTemperature;       // 165=16.5°C
    bool air3DMode;
    bool cleaningRequest;
    bool lockMode;
    bool turboMode;
    uint8_t fanSpeed;
    bool powerMode;
    uint8_t currentRequestedOperationMode;
    bool quietMode;
    bool temperatureUnit;
    uint8_t temperatureSetPoint;
    uint8_t sweepModeLR;
    uint8_t sweepModeUD;
    bool displayMode;
    bool ionizerMode;
    bool auxiliaryHeaterMode;
    uint8_t temperatureDisplayMode;
    bool sleepMode;
    bool energySavingMode;
    uint16_t remainingTimerMinutes;
    uint8_t currentOperationMode;
    uint16_t unitOperatingTimeHours;
    uint8_t heartBeat;
    uint8_t heartBeatValue;
    uint8_t isRemoreControlEnabled;

    uint16_t Debug2;
    uint16_t Debug1;
    uint8_t  Debug;

    UacDevice(Stream& serialPort, PubSubClient& pubSubClient, ESP8266WiFiClass& wifiClient,
              const char* topicPtr, char topicSizePtr = 0, uint16_t eepromStartAddress = 0x0400);

    void begin();
    void subscribeToTopics();
    uint8_t preFillConfiguration(uint8_t macBlocks[6]);
    void updateProgramsConfiguration(String targetParameter, uint64_t value, uint16_t program);
    void updateConfiguration(String targetParameter, uint16_t value);
    uint8_t decodeConfiguration();
    String  FrameToHexString(uint8_t FrameType);
    void    process(void);
    void    sendConfigurationFrame();
    void    sendHeartbeatFrame();
    void    heartBeatControl();

  private:
    char ValidFrame=0;
    Stream* serial;
    PubSubClient &mqtt;
    ESP8266WiFiClass &wifi;
    const char* topic_base;
    char topicSize;
    uint16_t eepromProgramStartAddress;

    enum FrameLengths {
      HEADER_FRAME_LENGTH = 10,
      A1_FRAME_LENGTH = 24,
      A2_FRAME_LENGTH = 12,
      A3_FRAME_LENGTH = 34,
      A4_FRAME_LENGTH = 13,
      A5_FRAME_LENGTH = 27,
      A6_FRAME_LENGTH = 28,
      AB_FRAME_LENGTH = 12,
      AC_FRAME_LENGTH = 18,
      CRC_LENGTH = 2
    };

    char FrameA1[A1_FRAME_LENGTH];
    char FrameA2[A2_FRAME_LENGTH];
    char FrameA3[A3_FRAME_LENGTH];
    char FrameA4[A4_FRAME_LENGTH];
    char FrameA5[A5_FRAME_LENGTH];
    char FrameA6[A6_FRAME_LENGTH];
    char FrameAB[AB_FRAME_LENGTH];
    char FrameAC[AC_FRAME_LENGTH];

    char Source = 0, Destination = 0, MessageLength = 0, UnknownHeader1 = 0, UnknownHeader2 = 0, FrameType = 0, Footer1 = 0, Footer2 = 0;

    enum Configuration {
        NUM_PROGRAMS = 16,
        NUM_PROGRAMS_NAME = 16,
        NUM_PROGRAMS_PARAMS = 8
    };

    enum Indices {
      A3_RETURN_AIR_TEMPERATURE_H = 10,
      A3_RETURN_AIR_TEMPERATURE_L = 11,
      A3_FUNCTIONNAL_INFORMATION = 12,
      A3_FAN_AND_OPERATION_MODE = 13,
      A3_QUIET_MODE_AND_SET_POINT = 14,
      A3_SWEEP_MODE = 15,
      A3_CONFIG_WORD = 16,
      A3_TIMER_L = 19,
      A3_TIMER_H = 20,
      A3_OPERATION_MODE_RETURN=21,
      A3_OPERATING_TIME_H=23,
      A3_OPERATING_TIME_L=24,
      A3_CRC_H=32,
      A3_CRC_L=33,

      A1_TIMER_L=10,
      A1_TIMER_H=11,
      A1_FAN_AND_OPERATION_MODE = 12,
      A1_QUIET_MODE_AND_SET_POINT = 13,
      A1_SWEEP_MODE = 14,
      A1_CONFIG_WORD = 15,
      A1_MAC_ADDRESS = 16,

      A4_FRAME_DATA = 10,
    };

    struct eepromPrograms {
      char    timerName[Configuration::NUM_PROGRAMS][Configuration::NUM_PROGRAMS_NAME];
      uint8_t activationProgram[Configuration::NUM_PROGRAMS][Configuration::NUM_PROGRAMS_PARAMS];
      uint8_t deactivationProgram[Configuration::NUM_PROGRAMS][Configuration::NUM_PROGRAMS_PARAMS];
      uint8_t areProgramsConfigured;
    };
    eepromPrograms MyEEPROMPrograms;

    uint16_t calculateCRC(const char* data, size_t length);
    void publishProgramsConfiguration();
    void publishFullConfiguration();
};
