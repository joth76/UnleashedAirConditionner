#pragma once
#include <Arduino.h>
#include <WiFi.h>
using ESP8266WiFiClass = WiFiClass;
#include <MD5Builder.h>
#include <Timezone.h>
#include <NTPClient.h>

// Convert a MAC address string into 6 bytes for frames, keeping ESP8266 compatibility.
void getMacAddressBlocks(ESP8266WiFiClass& wifi, uint8_t macBlocks[6]);

// Compute MD5 of a C string into a 16-byte array.
void calculateMD5(const char *input, uint8_t *md5Array);

// Daylight Saving helper for Central Europe (kept to preserve original behavior).
bool isDST(unsigned long epoch);

// Update an NTPClient with the proper CET/CEST offset (France time rules).
void updateNTPClientOffset(NTPClient& timeClient);

// Convert a single byte to a 2-digit lowercase hex string (e.g., 0x0A -> "0a").
String byteToHexString(char value);

// Convert an array of bytes to a space-separated hex string (e.g., "7a 7a d5 21 ...").
String arrayToHexString(char *arr, int length);

// Wi-Fi + MQTT helpers are kept in main to avoid coupling utils with user global clients.
