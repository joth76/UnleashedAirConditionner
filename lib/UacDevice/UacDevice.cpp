#include "UacDevice.h"

UacDevice::UacDevice(Stream& serialPort, PubSubClient& pubSubClient, ESP8266WiFiClass& wifiClient,
                     const char* topicPtr, char topicSizePtr, uint16_t eepromStartAddress)
  : serial(&serialPort), mqtt(pubSubClient), wifi(wifiClient), topic_base(topicPtr),
    topicSize(topicSizePtr), eepromProgramStartAddress(eepromStartAddress) {
  isConfigurationUpToDate = 0;
  sendConfigurationToAC = 0;

  returnAirTemperature = 0;
  cleaningRequest = 0;
  air3DMode = 0;
  lockMode = 0;
  turboMode = 0;
  fanSpeed = 0;
  powerMode = 0;
  currentRequestedOperationMode = 0;
  quietMode = 0;
  temperatureUnit = 0;
  temperatureSetPoint = 0;
  sweepModeLR = 0;
  sweepModeUD = 0;
  displayMode = 0;
  ionizerMode = 0;
  auxiliaryHeaterMode = 0;
  temperatureDisplayMode = 0;
  sleepMode = 0;
  energySavingMode = 0;
  remainingTimerMinutes = 0;
  currentOperationMode = 0;
  unitOperatingTimeHours = 0;
  heartBeat = 0xFF;
  heartBeatValue = 0;
  isRemoreControlEnabled = 0xFF;

  Debug = 0;
  Debug1 = 0;
  Debug2 = 0;
}

void UacDevice::begin() {
  Serial.printf("\r\nLoading Programs from EEPROM: ");
  EEPROM.begin(4096);
  EEPROM.get(eepromProgramStartAddress, MyEEPROMPrograms);
  EEPROM.end();

  if (MyEEPROMPrograms.areProgramsConfigured != 0x55) {
    for (int i = 0; i < Configuration::NUM_PROGRAMS; ++i) {
        strncpy(MyEEPROMPrograms.timerName[i], "Not Configured", Configuration::NUM_PROGRAMS_NAME -1);
        MyEEPROMPrograms.timerName[i][Configuration::NUM_PROGRAMS_NAME -1] = '\0';
        memset(MyEEPROMPrograms.activationProgram[i],   0xFF, sizeof(MyEEPROMPrograms.activationProgram[i]));
        memset(MyEEPROMPrograms.deactivationProgram[i], 0xFF, sizeof(MyEEPROMPrograms.deactivationProgram[i]));
    }
    MyEEPROMPrograms.areProgramsConfigured = 0x55;
    EEPROM.begin(4096);
    EEPROM.put(eepromProgramStartAddress, MyEEPROMPrograms);
    EEPROM.commit();
    EEPROM.end();
    Serial.printf(" Initialized !\r\n");
  } else {
    Serial.printf(" Loaded\r\n");
  } 
  publishProgramsConfiguration();
}

void UacDevice::subscribeToTopics() {
  String topic;
  topic = String(topic_base) + wifi.macAddress() + "/AC_config/#";
  mqtt.subscribe(topic.c_str());

  topic = String(topic_base) + wifi.macAddress() + "/AC_programs_config/#";
  mqtt.subscribe(topic.c_str());
}

uint8_t UacDevice::preFillConfiguration(uint8_t macBlocks[6]) {
  FrameA1[0] = 0x7A; FrameA1[1] = 0x7A; FrameA1[2] = 0x21; FrameA1[3] = 0xD5;
  FrameA1[4] = A1_FRAME_LENGTH; FrameA1[5] = 0x00; FrameA1[6] = 0x00; FrameA1[7] = 0xA1;
  FrameA1[8] = 0x00; FrameA1[9] = 0x00;

  FrameA1[A1_TIMER_L] = FrameA3[A3_TIMER_L];
  FrameA1[A1_TIMER_H] = FrameA3[A3_TIMER_H];
  FrameA1[A1_FAN_AND_OPERATION_MODE] = FrameA3[A3_FAN_AND_OPERATION_MODE];
  FrameA1[A1_QUIET_MODE_AND_SET_POINT] = FrameA3[A3_QUIET_MODE_AND_SET_POINT];
  FrameA1[A1_SWEEP_MODE] = FrameA3[A3_SWEEP_MODE];
  FrameA1[A1_CONFIG_WORD] = FrameA3[A3_CONFIG_WORD];

  for (int i=0;i<6;i++) FrameA1[A1_MAC_ADDRESS+i] = macBlocks[i];

  FrameA1[(A1_FRAME_LENGTH - 2)] = ((calculateCRC(FrameA1, A1_FRAME_LENGTH - 2) >> 8) & 0x00FF);
  FrameA1[(A1_FRAME_LENGTH - 1)] = (calculateCRC(FrameA1, A1_FRAME_LENGTH - 2) & 0x00FF );
  return isConfigurationUpToDate;
}

void UacDevice::updateProgramsConfiguration(String targetParameter, uint64_t value, uint16_t program) {
    if (program >= Configuration::NUM_PROGRAMS) return;

    if (targetParameter.startsWith("timerName")) {
        String newValue = String(value);
        newValue.toCharArray(MyEEPROMPrograms.timerName[program], Configuration::NUM_PROGRAMS_NAME);
    } else if (targetParameter.startsWith("activationProgram_")) {
        int paramIndex = targetParameter.substring(strlen("activationProgram_")).toInt();
        if (paramIndex >= 0 && paramIndex < Configuration::NUM_PROGRAMS_PARAMS) {
            MyEEPROMPrograms.activationProgram[program][paramIndex] = (uint8_t)value;
        }
    } else if (targetParameter.startsWith("deactivationProgram_")) {
        int paramIndex = targetParameter.substring(strlen("deactivationProgram_")).toInt();
        if (paramIndex >= 0 && paramIndex < Configuration::NUM_PROGRAMS_PARAMS) {
            MyEEPROMPrograms.deactivationProgram[program][paramIndex] = (uint8_t)value;
        }
    }
    
    EEPROM.begin(4096);
    EEPROM.put(eepromProgramStartAddress, MyEEPROMPrograms);
    EEPROM.commit();
    EEPROM.end();
    publishProgramsConfiguration();
}

void UacDevice::updateConfiguration(String targetParameter, uint16_t value) {
  if(targetParameter == "temperatureSetPoint") {
    FrameA1[A1_QUIET_MODE_AND_SET_POINT] = ( (FrameA1[A1_QUIET_MODE_AND_SET_POINT] & 0b11110000) | ((value - 16) & 0b00001111) );
  } else if(targetParameter == "operationMode") {
    FrameA1[A1_FAN_AND_OPERATION_MODE] = ( (FrameA1[A1_FAN_AND_OPERATION_MODE] & 0b11111000) | (value & 0b00000111) );
  } else if(targetParameter == "powerMode") {
    FrameA1[A1_FAN_AND_OPERATION_MODE] = ( (FrameA1[A1_FAN_AND_OPERATION_MODE] & 0b11110111) | ((value << 3) & 0b00001000) );
  } else if(targetParameter == "fanSpeed") {
    FrameA1[A1_FAN_AND_OPERATION_MODE] = ( (FrameA1[A1_FAN_AND_OPERATION_MODE] & 0b10001111) | ((value << 4) & 0b01110000) );
  } else if(targetParameter == "turboMode") {
    FrameA1[A1_FAN_AND_OPERATION_MODE] = ( (FrameA1[A1_FAN_AND_OPERATION_MODE] & 0b01111111) | ((value << 7) & 0b10000000) );
  } else if(targetParameter == "quietMode") {
    FrameA1[A1_QUIET_MODE_AND_SET_POINT] = ( (FrameA1[A1_QUIET_MODE_AND_SET_POINT] & 0b10111111) | ((value << 6) & 0b01000000) );
  } else if(targetParameter == "timer") {
    FrameA1[A1_TIMER_L] = ( FrameA1[A1_TIMER_L] | (value & 0x00FF)  );
    FrameA1[A1_TIMER_H] = ( FrameA1[A1_TIMER_H] | ((value >> 8) & 0x00FF) );
  } else if(targetParameter == "sweepModeLR") {
    FrameA1[A1_SWEEP_MODE] = ( (FrameA1[A1_SWEEP_MODE] & 0b00001111) | ((value << 4) & 0b11110000) );
  } else if(targetParameter == "sweepModeUD") {
    FrameA1[A1_SWEEP_MODE] = ( (FrameA1[A1_SWEEP_MODE] & 0b11110000) | (value & 0b00001111) );
  } else if(targetParameter == "temperatureUnit") {
    FrameA1[A1_QUIET_MODE_AND_SET_POINT] = ( (FrameA1[A1_QUIET_MODE_AND_SET_POINT] & 0b11011111) | ((value << 5) & 0b00100000) );
  } else if(targetParameter == "displayMode") {
    FrameA1[A1_CONFIG_WORD] = ( (FrameA1[A1_CONFIG_WORD] & 0b01111111) | ((value << 7) & 0b10000000) );
  } else if(targetParameter == "ionizerMode") {
    FrameA1[A1_CONFIG_WORD] = ( (FrameA1[A1_CONFIG_WORD] & 0b10111111) | ((value << 6) & 0b01000000) );
  } else if(targetParameter == "auxiliaryHeaterMode") {
    FrameA1[A1_CONFIG_WORD] = ( (FrameA1[A1_CONFIG_WORD] & 0b11101111) | ((value << 4) & 0b00010000) );
  } else if(targetParameter == "temperatureDisplayMode") {
    FrameA1[A1_CONFIG_WORD] = ( (FrameA1[A1_CONFIG_WORD] & 0b11110011) | ((value << 2) & 0b00001100) );
  } else if(targetParameter == "sleepMode") {
    FrameA1[A1_CONFIG_WORD] = ( (FrameA1[A1_CONFIG_WORD] & 0b11111101) | ((value << 1) & 0b00000010) );
  } else if(targetParameter == "energySavingMode") {
    FrameA1[A1_CONFIG_WORD] = ( (FrameA1[A1_CONFIG_WORD] & 0b11111110) | (value & 0b00000001) );
  }
  FrameA1[(A1_FRAME_LENGTH - 2)] = ((calculateCRC(FrameA1, A1_FRAME_LENGTH - 2) >> 8) & 0x00FF);
  FrameA1[(A1_FRAME_LENGTH - 1)] = (calculateCRC(FrameA1, A1_FRAME_LENGTH - 2) & 0x00FF );
}

uint8_t UacDevice::decodeConfiguration() {
  returnAirTemperature = (FrameA3[Indices::A3_RETURN_AIR_TEMPERATURE_H]*10) + FrameA3[Indices::A3_RETURN_AIR_TEMPERATURE_L];
  cleaningRequest = ((FrameA3[Indices::A3_FUNCTIONNAL_INFORMATION] >> 2) & 0b00000001);
  air3DMode = ((FrameA3[Indices::A3_FUNCTIONNAL_INFORMATION] >> 1) & 0b00000001);
  lockMode = (FrameA3[Indices::A3_FUNCTIONNAL_INFORMATION] & 0b00000001);
  turboMode = ((FrameA3[Indices::A3_FAN_AND_OPERATION_MODE] >> 7) & 0b00000001);
  fanSpeed = ((FrameA3[Indices::A3_FAN_AND_OPERATION_MODE] >> 4) & 0b00000111);
  powerMode = ((FrameA3[Indices::A3_FAN_AND_OPERATION_MODE] >> 3) & 0b00000001);
  currentRequestedOperationMode = (FrameA3[Indices::A3_OPERATION_MODE_RETURN] & 0b00000111);
  quietMode = ((FrameA3[Indices::A3_QUIET_MODE_AND_SET_POINT] >> 6) & 0b00000001);
  temperatureUnit = ((FrameA3[Indices::A3_QUIET_MODE_AND_SET_POINT] >> 5) & 0b00000001);
  temperatureSetPoint = (FrameA3[Indices::A3_QUIET_MODE_AND_SET_POINT] & 0b00001111) + 16;
  sweepModeLR = ((FrameA3[Indices::A3_SWEEP_MODE] >> 4) & 0b00001111);
  sweepModeUD = (FrameA3[Indices::A3_SWEEP_MODE] & 0b00001111);
  displayMode = ((FrameA3[Indices::A3_CONFIG_WORD] >> 7) & 0b00000001);
  ionizerMode = ((FrameA3[Indices::A3_CONFIG_WORD] >> 6) & 0b00000001);
  auxiliaryHeaterMode = ((FrameA3[Indices::A3_CONFIG_WORD] >> 4) & 0b00000001);
  temperatureDisplayMode = ((FrameA3[Indices::A3_CONFIG_WORD] >> 2) & 0b00000011);
  sleepMode = ((FrameA3[Indices::A3_CONFIG_WORD] >> 1) & 0b00000001);
  energySavingMode = (FrameA3[Indices::A3_CONFIG_WORD] & 0b00000001);
  remainingTimerMinutes = ((FrameA3[Indices::A3_TIMER_L] << 8) | (FrameA3[Indices::A3_TIMER_H]));
  currentOperationMode = ((FrameA3[Indices::A3_FAN_AND_OPERATION_MODE]) & 0b00000111) ;
  unitOperatingTimeHours = ((FrameA3[Indices::A3_OPERATING_TIME_H ] << 8) | (FrameA3[Indices::A3_OPERATING_TIME_L]));

  if ( ((FrameA3[Indices::A3_CRC_H ] << 8) | (FrameA3[Indices::A3_CRC_L])) == calculateCRC(FrameA3, 32) ) {
    isConfigurationUpToDate = 1;
  } else {
    isConfigurationUpToDate = 0;
  }
  return isConfigurationUpToDate;
}

String UacDevice::FrameToHexString(uint8_t FrameType) {
  String frameString;
  switch (FrameType) {
    case 0xA1: frameString = arrayToHexString(FrameA1, FrameLengths::A1_FRAME_LENGTH); break;
    case 0xA2: frameString = arrayToHexString(FrameA2, FrameLengths::A2_FRAME_LENGTH); break;
    case 0xA3: frameString = arrayToHexString(FrameA3, FrameLengths::A3_FRAME_LENGTH); break;
    case 0xA4: frameString = arrayToHexString(FrameA4, FrameLengths::A4_FRAME_LENGTH); break;
    case 0xA5: frameString = arrayToHexString(FrameA5, FrameLengths::A5_FRAME_LENGTH); break;
    case 0xA6: frameString = arrayToHexString(FrameA6, FrameLengths::A6_FRAME_LENGTH); break;
    case 0xAB: frameString = arrayToHexString(FrameAB, FrameLengths::AB_FRAME_LENGTH); break;
    case 0xAC: frameString = arrayToHexString(FrameAC, FrameLengths::AC_FRAME_LENGTH); break;
    default: break;
  }
  return frameString;
}

void UacDevice::sendConfigurationFrame() {
  serial->write(FrameA1, FrameLengths::A1_FRAME_LENGTH);
  serial->flush();
  sendConfigurationToAC = 0;
}

void UacDevice::process(void) {
  if(sendConfigurationToAC) {
    sendConfigurationFrame();
  }

  if(serial->available() > 0 ) {
    if(ValidFrame) {
      if (serial->available() >= (MessageLength - 10)) {
        ValidFrame = 0;
        Debug = FrameType;
        switch (FrameType) {
          case 0xA1:
            break;
          case 0xA2:
            break;
          case 0xA3:
            serial->readBytes(&FrameA3[10], 24);
            FrameA3[0] = 0x7A; FrameA3[1] = 0x7A; FrameA3[2] = Source; FrameA3[3] = Destination;
            FrameA3[4] = MessageLength; FrameA3[5] = UnknownHeader1; FrameA3[6] = UnknownHeader2;
            FrameA3[7] = FrameType; FrameA3[8] = Footer1; FrameA3[9] = Footer2;
            decodeConfiguration();
            publishFullConfiguration();
            break;
          case 0xA4:
            serial->readBytes(&FrameA4[10], 3);
            FrameA4[0] = 0x7A; FrameA4[1] = 0x7A; FrameA4[2] = Source; FrameA4[3] = Destination;
            FrameA4[4] = MessageLength; FrameA4[5] = UnknownHeader1; FrameA4[6] = UnknownHeader2;
            FrameA4[7] = FrameType; FrameA4[8] = Footer1; FrameA4[9] = Footer2;

            heartBeatValue = 32;
            if (FrameA4[Indices::A4_FRAME_DATA] == 0)       isRemoreControlEnabled = 1;
            else if (FrameA4[Indices::A4_FRAME_DATA] == 1)  isRemoreControlEnabled = 0;
            else if (FrameA4[Indices::A4_FRAME_DATA] == 0xA5) isRemoreControlEnabled = 2;
            break;
          case 0xA5:
            break;
          case 0xA6:
            serial->readBytes(&FrameA6[10], 18);
            FrameA6[0] = 0x7A; FrameA6[1] = 0x7A; FrameA6[2] = Source; FrameA6[3] = Destination;
            FrameA6[4] = MessageLength; FrameA6[5] = UnknownHeader1; FrameA6[6] = UnknownHeader2;
            FrameA6[7] = FrameType; FrameA6[8] = Footer1; FrameA6[9] = Footer2;
            break;
          case 0xAB:
            break;
          case 0xAC:
            serial->readBytes(&FrameAC[10], 8);
            FrameAC[0] = 0x7A; FrameAC[1] = 0x7A; FrameAC[2] = Source; FrameAC[3] = Destination;
            FrameAC[4] = MessageLength; FrameAC[5] = UnknownHeader1; FrameAC[6] = UnknownHeader2;
            FrameAC[7] = FrameType; FrameAC[8] = Footer1; FrameAC[9] = Footer2;
            break;
          default:
            break;
        }
      }
    } else {
      if(serial->available() >= 10 ) {
        Debug=0xff;
        if (serial->read() == 0x7A) {
          if (serial->read()== 0x7A) {
            Source = serial->read();
            Destination = serial->read();
            MessageLength = serial->read();
            UnknownHeader1 = serial->read();
            UnknownHeader2 = serial->read();
            FrameType = serial->read();
            Footer1 = serial->read();
            Footer2 = serial->read();
            ValidFrame = 1;
          }
        }
      }
    }
  }
}

void UacDevice::sendHeartbeatFrame() {
   FrameAB[0] = 0x7A;
   FrameAB[1] = 0x7A;
   FrameAB[2] = 0x21;
   FrameAB[3] = 0xD5;
   FrameAB[4] = AB_FRAME_LENGTH;
   FrameAB[5] = 0x00;
   FrameAB[6] = 0x00;
   FrameAB[7] = 0xAB;
   FrameAB[8] = 0x0A;
   FrameAB[9] = 0x0A;
   FrameAB[10] = ((calculateCRC(FrameAB, AB_FRAME_LENGTH - 2) >> 8) & 0x00FF);
   FrameAB[11] = (calculateCRC(FrameAB, AB_FRAME_LENGTH - 2) & 0x00FF );

   serial->write(FrameAB, FrameLengths::AB_FRAME_LENGTH);
   serial->flush();
}

void UacDevice::heartBeatControl() {
  if (heartBeatValue >= 1) { heartBeat = 1; heartBeatValue--; }
  else { heartBeat = 0; heartBeatValue = 0; }
}

uint16_t UacDevice::calculateCRC(const char* data, size_t length) {
  uint16_t crc = 0xFFFF;
  for (size_t i = 0; i < length; ++i) {
      crc ^= data[i];
      for (int j = 0; j < 8; ++j) {
          if (crc & 0x0001) {
              crc >>= 1;
              crc ^= 0xA001;
          } else {
              crc >>= 1;
          }
      }
  }
  return crc;
}

void UacDevice::publishProgramsConfiguration() {
    for (int i = 0; i < Configuration::NUM_PROGRAMS; ++i) {
        String timerNameTopic = String(topic_base) + wifi.macAddress() + "/AC_programs_status/" + String(i) + "/timerName";
        mqtt.publish(timerNameTopic.c_str(), MyEEPROMPrograms.timerName[i]);

        for (int j = 0; j < Configuration::NUM_PROGRAMS_PARAMS; ++j) {
            String activationProgramTopic = String(topic_base) + wifi.macAddress() + "/AC_programs_status/" + String(i) + "/activationProgram_" + String(j);
            mqtt.publish(activationProgramTopic.c_str(), String(MyEEPROMPrograms.activationProgram[i][j]).c_str());
        }
        for (int j = 0; j < Configuration::NUM_PROGRAMS_PARAMS; ++j) {
            String deactivationProgramTopic = String(topic_base) + wifi.macAddress() + "/AC_programs_status/" + String(i) + "/deactivationProgram_" + String(j);
            mqtt.publish(deactivationProgramTopic.c_str(), String(MyEEPROMPrograms.deactivationProgram[i][j]).c_str());
        }
    }
}

void UacDevice::publishFullConfiguration() {
    const String variablesToPublish[] = {
        "returnAirTemperature","cleaningRequest","air3DMode","lockMode","turboMode",
        "fanSpeed","powerMode","currentRequestedOperationMode","quietMode","temperatureUnit",
        "temperatureSetPoint","sweepModeLR","sweepModeUD","displayMode","ionizerMode",
        "auxiliaryHeaterMode","temperatureDisplayMode","sleepMode","energySavingMode",
        "remainingTimerMinutes","currentOperationMode","unitOperatingTimeHours"
    };

    for (const String& variable : variablesToPublish) {
        String topic = String(topic_base) + wifi.macAddress() + "/AC_status/" + variable;
        if (variable == "returnAirTemperature")       mqtt.publish(topic.c_str(), String(returnAirTemperature).c_str());
        else if (variable == "cleaningRequest")       mqtt.publish(topic.c_str(), String(cleaningRequest).c_str());
        else if (variable == "air3DMode")             mqtt.publish(topic.c_str(), String(air3DMode).c_str());
        else if (variable == "lockMode")              mqtt.publish(topic.c_str(), String(lockMode).c_str());
        else if (variable == "turboMode")             mqtt.publish(topic.c_str(), String(turboMode).c_str());
        else if (variable == "fanSpeed")              mqtt.publish(topic.c_str(), String(fanSpeed).c_str());
        else if (variable == "powerMode")             mqtt.publish(topic.c_str(), String(powerMode).c_str());
        else if (variable == "currentRequestedOperationMode") mqtt.publish(topic.c_str(), String(currentRequestedOperationMode).c_str());
        else if (variable == "quietMode")             mqtt.publish(topic.c_str(), String(quietMode).c_str());
        else if (variable == "temperatureUnit")       mqtt.publish(topic.c_str(), String(temperatureUnit).c_str());
        else if (variable == "temperatureSetPoint")   mqtt.publish(topic.c_str(), String(temperatureSetPoint).c_str());
        else if (variable == "sweepModeLR")           mqtt.publish(topic.c_str(), String(sweepModeLR).c_str());
        else if (variable == "sweepModeUD")           mqtt.publish(topic.c_str(), String(sweepModeUD).c_str());
        else if (variable == "displayMode")           mqtt.publish(topic.c_str(), String(displayMode).c_str());
        else if (variable == "ionizerMode")           mqtt.publish(topic.c_str(), String(ionizerMode).c_str());
        else if (variable == "auxiliaryHeaterMode")   mqtt.publish(topic.c_str(), String(auxiliaryHeaterMode).c_str());
        else if (variable == "temperatureDisplayMode")mqtt.publish(topic.c_str(), String(temperatureDisplayMode).c_str());
        else if (variable == "sleepMode")             mqtt.publish(topic.c_str(), String(sleepMode).c_str());
        else if (variable == "energySavingMode")      mqtt.publish(topic.c_str(), String(energySavingMode).c_str());
        else if (variable == "remainingTimerMinutes") mqtt.publish(topic.c_str(), String(remainingTimerMinutes).c_str());
        else if (variable == "currentOperationMode")  mqtt.publish(topic.c_str(), String(currentOperationMode).c_str());
        else if (variable == "unitOperatingTimeHours")mqtt.publish(topic.c_str(), String(unitOperatingTimeHours).c_str());
    }
}
