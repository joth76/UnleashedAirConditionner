#pragma once
#include <Arduino.h>
#include <EEPROM.h>
#include <WiFi.h>
using ESP8266WiFiClass = WiFiClass;

class SystemConfiguration {
  private:
    enum Flash_Words {
      FLASH_INITIALIZED = 0x5555,
      WIFI_CONFIG_UPDATED = 0xAAAA
    };
    static const int MAX_SSID_SIZE = 32;
    static const int MAX_PASSWORD_SIZE = 64;
    static const int MAX_HOSTNAME_SIZE = 32;
    static const int MAX_LOGIN_SIZE = 32;
    static const int ONBOARD_LED = 2;

  public:
    uint16_t FlashStatus;
    uint16_t WifiConfig;
    char wifi_ssid[MAX_SSID_SIZE];
    char wifi_password[MAX_PASSWORD_SIZE];
    char old_wifi_ssid[MAX_SSID_SIZE];
    char old_wifi_password[MAX_PASSWORD_SIZE];
    char ota_password[MAX_PASSWORD_SIZE];
    char hostname[MAX_HOSTNAME_SIZE];
    char http_login[MAX_LOGIN_SIZE];
    char http_password[MAX_PASSWORD_SIZE];
    char mqtt_login[MAX_LOGIN_SIZE];
    char mqtt_password[MAX_PASSWORD_SIZE];
    char mqtt_host[MAX_HOSTNAME_SIZE];
    uint16_t mqtt_port;

    SystemConfiguration();

    void flash_led(uint8_t repeat, uint8_t blinks);
    void load();
    void save();
    void reset();
};
extern SystemConfiguration config;
