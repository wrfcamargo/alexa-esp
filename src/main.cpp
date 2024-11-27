/*
 * Simple example for how to use multiple SinricPro Switch device:
 * - setup 4 switch devices
 * - handle request using multiple callbacks
 * 
 * If you encounter any issues:
 * - check the readme.md at https://github.com/sinricpro/esp8266-esp32-sdk/blob/master/README.md
 * - ensure all dependent libraries are installed
 *   - see https://github.com/sinricpro/esp8266-esp32-sdk/blob/master/README.md#arduinoide
 *   - see https://github.com/sinricpro/esp8266-esp32-sdk/blob/master/README.md#dependencies
 * - open serial monitor and check whats happening
 * - check full user documentation at https://sinricpro.github.io/esp8266-esp32-sdk
 * - visit https://github.com/sinricpro/esp8266-esp32-sdk/issues and check for existing issues or open a new one
 */

#ifdef ENABLE_DEBUG
   #define DEBUG_ESP_PORT Serial
   #define NODEBUG_WEBSOCKETS
   #define NDEBUG
#endif

#include <Arduino.h>
#if defined(ESP8266)
  #include <ESP8266WiFi.h>
#elif defined(ESP32) || defined(ARDUINO_ARCH_RP2040)
  #include <WiFi.h>
#endif

#include "SinricPro.h"
#include "SinricProSwitch.h"

#define WIFI_SSID         "Camargo 2.4G"
#define WIFI_PASS         "AW12012019"
#define APP_KEY           "9c1b45d6-86f1-48b1-96e6-5ddfffa811e8"
#define APP_SECRET        "470af8d5-ef5f-41ff-bd25-aefa6322d1d0-0d55eff4-05e0-4292-8768-5298126c1bcc"

#define SWITCH_ID_1       "674676c68916d6b80a231abc"
#define RELAYPIN_1        1

#define BAUD_RATE         115200

bool onPowerState1(const String &deviceId, bool &state)
{
    Serial.printf("Device 1 turned %s\r\n", state ? "on" : "off");
    // digitalWrite(RELAYPIN_1, state ? HIGH:LOW);
    return true;
}

void setupWiFi()
{
    Serial.println("[Wifi]: Connecting");

    #if defined(ESP8266)
        WiFi.setSleepMode(WIFI_NONE_SLEEP); 
        WiFi.setAutoReconnect(true);
    #elif defined(ESP32)
        WiFi.setSleep(false); 
        WiFi.setAutoReconnect(true);
    #endif

    WiFi.begin(WIFI_SSID, WIFI_PASS);

    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(1000);
    }

    Serial.printf("connected!\r\n[WiFi]: IP-Address is %s\r\n", WiFi.localIP().toString().c_str());
}

void setupSinricPro(void)
{
    // pinMode(RELAYPIN_1, OUTPUT);

    SinricProSwitch& mySwitch1 = SinricPro[SWITCH_ID_1];
    mySwitch1.onPowerState(onPowerState1);

    SinricPro.onConnected([](){ Serial.print("Connected to SinricPro\r\n"); });
    SinricPro.onDisconnected([](){ Serial.print("Disconnected from SinricPro\r\n"); });

    SinricPro.begin(APP_KEY, APP_SECRET);
}

void setup()
{
    Serial.begin(BAUD_RATE);
    Serial.print("\r\n\r\n");
    setupWiFi();
    setupSinricPro();
}

void loop()
{
    SinricPro.handle();
}
