#pragma once
#include <ESPAsyncWebServer.h>

void startServers();
void handleRoot(AsyncWebServerRequest *request);
void handleCSS(AsyncWebServerRequest *request);
void handleModuleConfiguration(AsyncWebServerRequest *request);
void handleReadConfiguration(AsyncWebServerRequest *request);
void handleWriteConfiguration(AsyncWebServerRequest *request);
void handlePushConfigurationToAC(AsyncWebServerRequest *request);
void handleDebugInformation(AsyncWebServerRequest *request);
void handleDoUpdate(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final);
void handleWifi(AsyncWebServerRequest *request);
void handleReboot(AsyncWebServerRequest *request);
void handleAcStatus(AsyncWebServerRequest *request);
void handleAcControl(AsyncWebServerRequest *request);
