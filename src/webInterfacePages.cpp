#include <Arduino.h>
#include "htmlCode.h"
#include "SystemConfiguration.h"
#include "Utils.h"
#include "UacDevice.h"
#include <Update.h>
#include <ESPAsyncWebServer.h>

extern AsyncWebServer server;
extern UacDevice MyUacDevice;
extern SystemConfiguration config;

// Function to generate HTML for displaying status dots and text
String generateStatusHTML(const String& variableName, int statusValue, int redValue = 0, int greenValue = 1, int yellowValue = 2, int orangeValue = 3) {
    String statusDotClass = "status-dot ";
    String statusText = "";

    if (statusValue == redValue) {
        statusDotClass += "red-dot";
    } else if (statusValue == greenValue) {
        statusDotClass += "green-dot";
    } else if (statusValue == yellowValue) {
        statusDotClass += "yellow-dot";
    } else if (statusValue == orangeValue) {
        statusDotClass += "orange-dot";
    } else {
        statusDotClass += "grey-dot";
    }

    String html = "<div class='status-container'>"
                 "<span class='" + statusDotClass + "'></span>"
                 "<span class='status-text'>" + variableName + "</span>"
                 "</div>";

    return html;
}

String variablesSubstitution(const String& var) {
  String Answer;

  if (var == "COPYRIGHT") {
      Answer="<footer><p>&copy; 2023 TheMiNuS</p></footer>";
  } else if (var == "isConfigurationUpToDate") {
    Answer = generateStatusHTML("Configuration Status", MyUacDevice.isConfigurationUpToDate);
  } else if (var == "isRemoreControlEnabled") {
    Answer = generateStatusHTML("Remotecontrol Activation", MyUacDevice.isRemoreControlEnabled);
  } else if (var == "heartBeat") {
    Answer = generateStatusHTML("Heartbeat", MyUacDevice.heartBeat); 
  } else if (var == "wifi_ssid") {
    Answer = String(config.wifi_ssid);
  } else if (var == "wifi_password") {
    Answer = String(config.wifi_password);
  } else if (var == "http_login") {
    Answer = String(config.http_login);
  } else if (var == "http_password") {
    Answer = String(config.http_password);
  } else if (var == "hostname") {
    Answer = String(config.hostname);
  } else if (var == "ota_password") {
    Answer = String(config.ota_password);
  } else if (var == "mqtt_login") {
    Answer = String(config.mqtt_login);
  } else if (var == "mqtt_password") {
    Answer = String(config.mqtt_password);
  } else if (var == "mqtt_host") {
    Answer = String(config.mqtt_host);
  } else if (var == "mqtt_port") {
    Answer = String(config.mqtt_port);
  } else if (var == "returnAirTemperature") {
    Answer = String(MyUacDevice.returnAirTemperature);
  } else if (var == "operationalMode") {
    Answer = String(MyUacDevice.currentOperationMode);
  } else if (var == "air3DMode") {
    Answer = String(MyUacDevice.air3DMode);
  } else if (var == "cleaningRequest") {
    Answer = String(MyUacDevice.cleaningRequest);
  } else if (var == "lockMode") {
    Answer = String(MyUacDevice.lockMode);
  } else if (var == "turboMode") {
    Answer = String(MyUacDevice.turboMode);
  } else if (var == "fanSpeed") {
    Answer = String(MyUacDevice.fanSpeed);
  } else if (var == "powerMode") {
    Answer = String(MyUacDevice.powerMode);
  } else if (var == "currentRequestedOperationMode") {
    Answer = String(MyUacDevice.currentRequestedOperationMode);
  } else if (var == "quietMode") {
    Answer = String(MyUacDevice.quietMode);
  } else if (var == "temperatureUnit") {
    Answer = String(MyUacDevice.temperatureUnit);
  } else if (var == "temperatureSetPoint") {
    Answer = String(MyUacDevice.temperatureSetPoint);
  } else if (var == "sweepModeLR") {
    Answer = String(MyUacDevice.sweepModeLR);
  } else if (var == "sweepModeUD") {
    Answer = String(MyUacDevice.sweepModeUD);
  } else if (var == "displayMode") {
    Answer = String(MyUacDevice.displayMode);
  } else if (var == "ionizerMode") {
    Answer = String(MyUacDevice.ionizerMode);
  } else if (var == "auxiliaryHeaterMode") {
    Answer = String(MyUacDevice.auxiliaryHeaterMode);
  } else if (var == "temperatureDisplayMode") {
    Answer = String(MyUacDevice.temperatureDisplayMode);
  } else if (var == "sleepMode") {
    Answer = String(MyUacDevice.sleepMode);
  } else if (var == "energySavingMode") {
    Answer = String(MyUacDevice.energySavingMode);
  } else if (var == "remainingTimerMinutes") {
    Answer = String(MyUacDevice.remainingTimerMinutes);
  } else if (var == "currentOperationMode") {
    Answer = String(MyUacDevice.currentOperationMode);
  } else if (var == "unitOperatingTimeHours") {
    Answer = String(MyUacDevice.unitOperatingTimeHours);
  } else if (var == "isConfigurationUpToDate") {
    Answer = String(MyUacDevice.isConfigurationUpToDate);
  } else if (var == "FrameToHexStringA1") {
    Answer = MyUacDevice.FrameToHexString(0xA1);
  } else if (var == "FrameToHexStringA2") {
    Answer = MyUacDevice.FrameToHexString(0xA2);
  } else if (var == "FrameToHexStringA3") {
    Answer = MyUacDevice.FrameToHexString(0xA3);
  } else if (var == "FrameToHexStringA4") {
    Answer = MyUacDevice.FrameToHexString(0xA4);
  } else if (var == "FrameToHexStringA5") {
    Answer = MyUacDevice.FrameToHexString(0xA5);
  } else if (var == "FrameToHexStringA6") {
    Answer = MyUacDevice.FrameToHexString(0xA6);
  } else if (var == "FrameToHexStringAB") {
    Answer = MyUacDevice.FrameToHexString(0xAB);
  } else if (var == "FrameToHexStringAC") {
    Answer = MyUacDevice.FrameToHexString(0xAC);
  } else if (var == "SETPOINT_VALUES") {
    for (uint8_t i=16; i <= 31; i++) {
      Answer = Answer + "<option value='" + String(i) + "'" + (MyUacDevice.temperatureSetPoint == i ? " selected" : "") + ">" + String(i) + "°C </option>\n";
    }
  } else if (var == "OPERATIONAL_MODE") {
    Answer = Answer + "<option value='0'" + (MyUacDevice.currentOperationMode == 0 ? " selected" : "") + ">Automatic</option>\n";
    Answer = Answer + "<option value='1'" + (MyUacDevice.currentOperationMode == 1 ? " selected" : "") + ">Cooling</option>\n";
    Answer = Answer + "<option value='2'" + (MyUacDevice.currentOperationMode == 2 ? " selected" : "") + ">Drying</option>\n";
    Answer = Answer + "<option value='3'" + (MyUacDevice.currentOperationMode == 3 ? " selected" : "") + ">Ventilation</option>\n";
    Answer = Answer + "<option value='4'" + (MyUacDevice.currentOperationMode == 4 ? " selected" : "") + ">Heating</option>\n";
  } else if (var == "POWER_MODE") {
    Answer = Answer + "<option value='0'" + (MyUacDevice.powerMode == 0 ? " selected" : "") + ">Off</option>\n";
    Answer = Answer + "<option value='1'" + (MyUacDevice.powerMode == 1 ? " selected" : "") + ">On</option>\n";
  } else if (var == "FAN_SPEED") {
    Answer = Answer + "<option value='0'" + (MyUacDevice.fanSpeed == 0 ? " selected" : "") + ">Automatic</option>\n";
    Answer = Answer + "<option value='1'" + (MyUacDevice.fanSpeed == 1 ? " selected" : "") + ">Speed1</option>\n";
    Answer = Answer + "<option value='2'" + (MyUacDevice.fanSpeed == 2 ? " selected" : "") + ">Speed2</option>\n";
    Answer = Answer + "<option value='3'" + (MyUacDevice.fanSpeed == 3 ? " selected" : "") + ">Speed3</option>\n";
    Answer = Answer + "<option value='4'" + (MyUacDevice.fanSpeed == 4 ? " selected" : "") + ">Speed4</option>\n";
    Answer = Answer + "<option value='5'" + (MyUacDevice.fanSpeed == 5 ? " selected" : "") + ">Speed5</option>\n";
    Answer = Answer + "<option value='6'" + (MyUacDevice.fanSpeed == 6 ? " selected" : "") + ">Stepless</option>\n";
  } else if (var == "TURBO_MODE") {
    Answer = Answer + "<option value='0'" + (MyUacDevice.turboMode == 0 ? " selected" : "") + ">Off</option>\n";
    Answer = Answer + "<option value='1'" + (MyUacDevice.turboMode == 1 ? " selected" : "") + ">On</option>\n";
  } else if (var == "QUIET_MODE") {
    Answer = Answer + "<option value='0'" + (MyUacDevice.quietMode == 0 ? " selected" : "") + ">Off</option>\n";
    Answer = Answer + "<option value='1'" + (MyUacDevice.quietMode == 1 ? " selected" : "") + ">On</option>\n";
  } else if (var == "TIMER_VALUE") {
    Answer = String(MyUacDevice.remainingTimerMinutes);
  } else if (var == "SWEEPLR") {
    Answer = Answer + "<option value='0'" + (MyUacDevice.sweepModeLR == 0 ? " selected" : "") + ">Off</option>\n";
    Answer = Answer + "<option value='1'" + (MyUacDevice.sweepModeLR == 1 ? " selected" : "") + ">Static</option>\n";
    Answer = Answer + "<option value='2'" + (MyUacDevice.sweepModeLR == 2 ? " selected" : "") + ">Position 1</option>\n";
    Answer = Answer + "<option value='3'" + (MyUacDevice.sweepModeLR == 3 ? " selected" : "") + ">Position 2</option>\n";
    Answer = Answer + "<option value='4'" + (MyUacDevice.sweepModeLR == 4 ? " selected" : "") + ">Position 3</option>\n";
  } else if (var == "SWEEPUD") {
    Answer = Answer + "<option value='0'" + (MyUacDevice.sweepModeUD == 0 ? " selected" : "") + ">Off</option>\n";
    Answer = Answer + "<option value='1'" + (MyUacDevice.sweepModeUD == 1 ? " selected" : "") + ">Sweep 1 to 5</option>\n";
    Answer = Answer + "<option value='2'" + (MyUacDevice.sweepModeUD == 3 ? " selected" : "") + ">Position 1</option>\n";
    Answer = Answer + "<option value='3'" + (MyUacDevice.sweepModeUD == 3 ? " selected" : "") + ">Position 2</option>\n";
    Answer = Answer + "<option value='4'" + (MyUacDevice.sweepModeUD == 4 ? " selected" : "") + ">Position 3</option>\n";
    Answer = Answer + "<option value='5'" + (MyUacDevice.sweepModeUD == 5 ? " selected" : "") + ">Position 4</option>\n";
    Answer = Answer + "<option value='6'" + (MyUacDevice.sweepModeUD == 6 ? " selected" : "") + ">Position 5</option>\n";
    Answer = Answer + "<option value='7'" + (MyUacDevice.sweepModeUD == 7 ? " selected" : "") + ">Sweep 3 to 5</option>\n";
    Answer = Answer + "<option value='8'" + (MyUacDevice.sweepModeUD == 8 ? " selected" : "") + ">Sweep 2 to 5</option>\n";
    Answer = Answer + "<option value='9'" + (MyUacDevice.sweepModeUD == 9 ? " selected" : "") + ">Sweep 2 to 4</option>\n";
    Answer = Answer + "<option value='10'" + (MyUacDevice.sweepModeUD == 10 ? " selected" : "") + ">Sweep 1 to 4</option>\n";
    Answer = Answer + "<option value='11'" + (MyUacDevice.sweepModeUD == 11 ? " selected" : "") + ">Sweep 1 to 3</option>\n";
    Answer = Answer + "<option value='12'" + (MyUacDevice.sweepModeUD == 12 ? " selected" : "") + ">Sweep 4 to 6</option>\n";
    Answer = Answer + "<option value='13'" + (MyUacDevice.sweepModeUD == 13 ? " selected" : "") + ">Position 6</option>\n";
  } else if (var == "TEMPERATURE_UNIT") {
    Answer = Answer + "<option value='0'" + (MyUacDevice.temperatureUnit == 0 ? " selected" : "") + ">Celcius</option>\n";
    Answer = Answer + "<option value='1'" + (MyUacDevice.temperatureUnit == 1 ? " selected" : "") + ">Fahreheit</option>\n";
  } else if (var == "DISPLAY_MODE") {
    Answer = Answer + "<option value='0'" + (MyUacDevice.displayMode == 0 ? " selected" : "") + ">Off</option>\n";
    Answer = Answer + "<option value='1'" + (MyUacDevice.displayMode == 1 ? " selected" : "") + ">On</option>\n";
  } else if (var == "IONIZER_MODE") {
    Answer = Answer + "<option value='0'" + (MyUacDevice.ionizerMode == 0 ? " selected" : "") + ">Off</option>\n";
    Answer = Answer + "<option value='1'" + (MyUacDevice.ionizerMode == 1 ? " selected" : "") + ">On</option>\n";
  } else if (var == "AUXILIARY_HEATER") {
    Answer = Answer + "<option value='0'" + (MyUacDevice.auxiliaryHeaterMode == 0 ? " selected" : "") + ">Off</option>\n";
    Answer = Answer + "<option value='1'" + (MyUacDevice.auxiliaryHeaterMode == 1 ? " selected" : "") + ">On</option>\n";
  } else if (var == "TEMPERATURE_DISPLAY") {
    Answer = Answer + "<option value='0'" + (MyUacDevice.temperatureDisplayMode == 0 ? " selected" : "") + ">Default ?</option>\n";
    Answer = Answer + "<option value='1'" + (MyUacDevice.temperatureDisplayMode == 1 ? " selected" : "") + ">Set point ?</option>\n";
    Answer = Answer + "<option value='2'" + (MyUacDevice.temperatureDisplayMode == 2 ? " selected" : "") + ">Indoor ambient ?</option>\n";
    Answer = Answer + "<option value='3'" + (MyUacDevice.temperatureDisplayMode == 3 ? " selected" : "") + ">Outdoor ambient ?</option>\n";
  } else if (var == "ENERGY_SAVING") {
    Answer = Answer + "<option value='0'" + (MyUacDevice.energySavingMode == 0 ? " selected" : "") + ">Off</option>\n";
    Answer = Answer + "<option value='1'" + (MyUacDevice.energySavingMode == 1 ? " selected" : "") + ">On</option>\n";
  } else if (var == "SLEEP_MODE") {
    Answer = Answer + "<option value='0'" + (MyUacDevice.sleepMode == 0 ? " selected" : "") + ">Off</option>\n";
    Answer = Answer + "<option value='1'" + (MyUacDevice.sleepMode == 1 ? " selected" : "") + ">On</option>\n";
  }

  return String(Answer);
}

void handleRoot(AsyncWebServerRequest *request) {
   if (!request->authenticate(config.http_login, config.http_password ))
     return request->requestAuthentication();

   request->send_P(200, "text/html", HTML_ROOT, variablesSubstitution);
}

void handleCSS(AsyncWebServerRequest *request) {
  request->send(200, "text/css", HTML_CSS_STYLE);
}

void handleModuleConfiguration(AsyncWebServerRequest *request) {
   if (!request->authenticate(config.http_login, config.http_password ))
     return request->requestAuthentication();

   request->send_P(200, "text/html", HTML_MODULE_CONFIGURATION, variablesSubstitution);
}

void handleReadConfiguration(AsyncWebServerRequest *request) {
   if (!request->authenticate(config.http_login, config.http_password ))
     return request->requestAuthentication();

   request->send_P(200, "text/html", HTML_READ_CONFIGURATION, variablesSubstitution);
}

void handleWriteConfiguration(AsyncWebServerRequest *request) {
  if (!request->authenticate(config.http_login, config.http_password ))
     return request->requestAuthentication();
  if (MyUacDevice.isConfigurationUpToDate) {
    request->send_P(200, "text/html", HTML_WRITE_CONFIGURATION, variablesSubstitution);
  } else {
    request->send_P(200, "text/html", HTML_WAIT_FOR_VALID_CONFIGURATION, variablesSubstitution);
  }
}

void handlePushConfigurationToAC(AsyncWebServerRequest *request) {
   if (!request->authenticate(config.http_login, config.http_password ))
     return request->requestAuthentication();

  uint8_t macBlocks[6] = { 0, 0, 0, 0, 0, 0};
  getMacAddressBlocks(WiFi, macBlocks);
  MyUacDevice.preFillConfiguration(macBlocks);

  if(request->hasParam("temperatureSetPoint")) {
    MyUacDevice.updateConfiguration("temperatureSetPoint" , request->getParam("temperatureSetPoint")->value().toInt());
  }
  if(request->hasParam("operationMode")) {
    MyUacDevice.updateConfiguration("operationMode" , request->getParam("operationMode")->value().toInt());
  }
  if(request->hasParam("powerMode")) {
    MyUacDevice.updateConfiguration("powerMode" , request->getParam("powerMode")->value().toInt());
  }
  if(request->hasParam("fanSpeed")) {
    MyUacDevice.updateConfiguration("fanSpeed" , request->getParam("fanSpeed")->value().toInt());
  }
  if(request->hasParam("turboMode")) {
    MyUacDevice.updateConfiguration("turboMode" , request->getParam("turboMode")->value().toInt());
  }
  if(request->hasParam("quietMode")) {
    MyUacDevice.updateConfiguration("quietMode" , request->getParam("quietMode")->value().toInt());
  }
  if(request->hasParam("timer")) {
    MyUacDevice.updateConfiguration("timer" , request->getParam("timer")->value().toInt());
  }
  if(request->hasParam("sweepModeLR")) {
    MyUacDevice.updateConfiguration("sweepModeLR" , request->getParam("sweepModeLR")->value().toInt());
  }
  if(request->hasParam("sweepModeUD")) {
    MyUacDevice.updateConfiguration("sweepModeUD" , request->getParam("sweepModeUD")->value().toInt());
  }
  if(request->hasParam("temperatureUnit")) {
    MyUacDevice.updateConfiguration("temperatureUnit" , request->getParam("temperatureUnit")->value().toInt());
  }
  if(request->hasParam("displayMode")) {
    MyUacDevice.updateConfiguration("displayMode" , request->getParam("displayMode")->value().toInt());
  }
  if(request->hasParam("ionizerMode")) {
    MyUacDevice.updateConfiguration("ionizerMode" , request->getParam("ionizerMode")->value().toInt());
  }
  if(request->hasParam("auxiliaryHeaterMode")) {
    MyUacDevice.updateConfiguration("auxiliaryHeaterMode" , request->getParam("auxiliaryHeaterMode")->value().toInt());
  }
  if(request->hasParam("temperatureDisplayMode")) {
    MyUacDevice.updateConfiguration("temperatureDisplayMode" , request->getParam("temperatureDisplayMode")->value().toInt());
  }
  if(request->hasParam("energySavingMode")) {
    MyUacDevice.updateConfiguration("energySavingMode" , request->getParam("energySavingMode")->value().toInt());
  }
  if(request->hasParam("sleepMode")) {
    MyUacDevice.updateConfiguration("sleepMode" , request->getParam("sleepMode")->value().toInt());
  }

  MyUacDevice.sendConfigurationToAC = 1;
  MyUacDevice.sendConfigurationFrame();
  request->send_P(200, "text/html", HTML_PUSH_CONFIGURATION_TO_AC, variablesSubstitution);
}

void handleDebugInformation(AsyncWebServerRequest *request) {
   if (!request->authenticate(config.http_login, config.http_password ))
     return request->requestAuthentication();

  request->send_P(200, "text/html", HTML_DEBUG_INFORMATION, variablesSubstitution);
}

// handleDoUpdate based on lbernstone https://github.com/lbernstone/asyncUpdate
void handleDoUpdate(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final) {
    extern size_t content_len;
    if (!index) {
        content_len = request->contentLength();
#if defined(ESP8266)
        Update.runAsync(true);
#endif
        if (!Update.begin(content_len)) {
            request->send(200, "text/html", HTML_FIRMWARE_UPGRADE_ERROR);
        }
    }
    if (Update.write(data, len) != len) { /* handle error if needed */ }
    if (final) {
        if (!Update.end(true)) {
            request->send(200, "text/html", HTML_FIRMWARE_UPGRADE_ERROR);
        } else {
            request->send(200, "text/html", HTML_FIRMWARE_UPGRADE_SUCCESSFULL);
        }
    }
}

void handleWifi(AsyncWebServerRequest *request) {
   if (!request->authenticate(config.http_login, config.http_password ))
     return request->requestAuthentication();

  config.WifiConfig = 0xAAAA;
  strcpy(config.old_wifi_ssid, config.wifi_ssid);
  strcpy(config.old_wifi_password, config.wifi_password);

  if(request->hasParam("wifiSSID")) {
    String ssid = request->arg("wifiSSID");
    strcpy(config.wifi_ssid, ssid.c_str());
  }
  if(request->hasParam("wifiPassword")) {
    String password = request->arg("wifiPassword");
    strcpy(config.wifi_password, password.c_str());
  }
  if(request->hasParam("httpLogin")) {
    String login = request->arg("httpLogin");
    strcpy(config.http_login, login.c_str());
  }
  if(request->hasParam("httpPassword")) {
    String httpPassword = request->arg("httpPassword");
    strcpy(config.http_password, httpPassword.c_str());
  }
  if(request->hasParam("hostname")) {
    String host = request->arg("hostname");
    strcpy(config.hostname, host.c_str());
  }
  if(request->hasParam("mqttLogin")) {
    String mqttLogin = request->arg("mqttLogin");
    strcpy(config.mqtt_login, mqttLogin.c_str());
  }
  if(request->hasParam("mqttPassword")) {
    String mqttPassword = request->arg("mqttPassword");
    strcpy(config.mqtt_password, mqttPassword.c_str());
  }
  if(request->hasParam("mqttHost")) {
    String mqttHost = request->arg("mqttHost");
    strcpy(config.mqtt_host, mqttHost.c_str());
  }
  if(request->hasParam("mqttPort")) {
    config.mqtt_port = request->getParam("mqttPort")->value().toInt();
  }
  if (request->hasParam("otaPassword")) {
    String plain = request->arg("otaPassword");

    if (plain.length() == 0) {
      // Empty field disables OTA (startOTA() already checks for empty hash)
      strcpy(config.ota_password, "");
    } else {
      uint8_t md5[16];
      calculateMD5(plain.c_str(), md5);  // from Utils.h
      // Convert 16-byte MD5 to 32-char lowercase hex
      String hex = "";
      for (int i = 0; i < 16; i++) {
        hex += byteToHexString((char)md5[i]);  // from Utils.h
      }
      // Persist hash in EEPROM-backed config
      strncpy(config.ota_password, hex.c_str(), sizeof(config.ota_password) - 1);
      config.ota_password[sizeof(config.ota_password) - 1] = '\0';
    }
  }

  config.save();
  request->send_P(200, "text/html", HTML_PUSH_CONFIGURATION_TO_MODULE, variablesSubstitution);
}

void handleReboot(AsyncWebServerRequest *request) {
   if (!request->authenticate(config.http_login, config.http_password ))
     return request->requestAuthentication();
  request->send(200, "text/html", "OK !");
  ESP.restart();
}

void handleAcStatus(AsyncWebServerRequest *request) {
  if (!request->authenticate(config.http_login, config.http_password ))
    return request->requestAuthentication();

  String payload = "{\"power\":" + String(MyUacDevice.powerMode ? 1 : 0) +
                   ",\"mode\":" + String(MyUacDevice.currentRequestedOperationMode) +
                   ",\"setpoint\":" + String(MyUacDevice.temperatureSetPoint) +
                   ",\"returnAirTemperature\":" + String(MyUacDevice.returnAirTemperature) +
                   "}";
  request->send(200, "application/json", payload);
}

void handleAcControl(AsyncWebServerRequest *request) {
  if (!request->authenticate(config.http_login, config.http_password ))
    return request->requestAuthentication();

  int power = -1;
  int mode = -1;
  int setpoint = -1;

  uint8_t macBlocks[6] = { 0, 0, 0, 0, 0, 0};
  getMacAddressBlocks(WiFi, macBlocks);
  MyUacDevice.preFillConfiguration(macBlocks);

  if (request->hasParam("power")) {
    power = request->getParam("power")->value().toInt();
    MyUacDevice.updateConfiguration("powerMode", power);
  }
  if (request->hasParam("mode")) {
    mode = request->getParam("mode")->value().toInt();
    MyUacDevice.updateConfiguration("operationMode", mode);
  }
  if (request->hasParam("setpoint")) {
    setpoint = request->getParam("setpoint")->value().toInt();
    MyUacDevice.updateConfiguration("temperatureSetPoint", setpoint);
  }

  MyUacDevice.sendConfigurationToAC = 1;
  MyUacDevice.sendConfigurationFrame();
  Serial.printf("AC API control: power=%d mode=%d setpoint=%d\r\n", power, mode, setpoint);
  request->send(200, "application/json", "{\"ok\":true}");
}

void startServers() {
   extern AsyncWebServer server;
   server.begin();
   server.on("/", HTTP_GET, handleRoot);
   server.on("/module-configuration", HTTP_GET, handleModuleConfiguration);
   server.on("/read-configuration", HTTP_GET, handleReadConfiguration);
   server.on("/write-configuration", HTTP_GET, handleWriteConfiguration);
   server.on("/push-configuration", HTTP_GET, handlePushConfigurationToAC);
   server.on("/debug-information", HTTP_GET, handleDebugInformation);
   server.on("/wifi", HTTP_GET, handleWifi);
   server.on("/reboot", HTTP_GET, handleReboot);
   server.on("/api/ac/status", HTTP_GET, handleAcStatus);
   server.on("/api/ac/control", HTTP_GET, handleAcControl);
   server.on("/styles.css", HTTP_GET, handleCSS);
   server.on("/doUpdate", HTTP_POST,
     [](AsyncWebServerRequest *request) {request->send(200, "text/html", HTML_FIRMWARE_UPGRADE_ERROR);},
     [](AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final) {
        handleDoUpdate(request, filename, index, data, len, final);
     }
   );
}
