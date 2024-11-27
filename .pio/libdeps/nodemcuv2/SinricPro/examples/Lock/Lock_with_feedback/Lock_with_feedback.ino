/*
 * Example for smart lock with feedback
 * 
 * The lock must give a feedback signal on pin defined in LOCK_STATE_PIN
 * Alternative: Use a contact sensor, indicating the current lock state
 * HIGH on LOCK_STATE_PIN means lock is locked
 * LOW on LOCK_STATE_PIN means lock is unlocked
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

// Uncomment the following line to enable serial debug output
//#define ENABLE_DEBUG

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
#include "SinricProLock.h"

#define WIFI_SSID         "YOUR_WIFI_SSID"    
#define WIFI_PASS         "YOUR_WIFI_PASSWORD"
#define APP_KEY           "YOUR_APP_KEY_HERE"      // Should look like "de0bxxxx-1x3x-4x3x-ax2x-5dabxxxxxxxx"
#define APP_SECRET        "YOUR_APP_SECRET_HERE"   // Should look like "5f36xxxx-x3x7-4x3x-xexe-e86724a9xxxx-4c4axxxx-3x3x-x5xe-x9x3-333d65xxxxxx"
#define LOCK_ID           "YOUR_DEVICE_ID_HERE"    // Should look like "5dc1564130xxxxxxxxxxxxxx"
#define BAUD_RATE         115200                     // Change baudrate to your need

#if defined(ESP8266)
  #define LOCK_PIN          D1                       // PIN where the lock is connected to: HIGH = locked, LOW = unlocked
  #define LOCK_STATE_PIN    D2                       // PIN where the lock feedback is connected to (HIGH:locked, LOW:unlocked)
#elif defined(ESP32) || defined(ARDUINO_ARCH_RP2040)
  #define LOCK_PIN          16                       // PIN where the lock is connected to: HIGH = locked, LOW = unlocked
  #define LOCK_STATE_PIN    17                       // PIN where the lock feedback is connected to (HIGH:locked, LOW:unlocked)
#endif

bool lastLockState;

bool onLockState(String deviceId, bool &lockState) {
  Serial.printf("Device %s is %s\r\n", deviceId.c_str(), lockState?"locked":"unlocked");
  digitalWrite(LOCK_PIN, lockState);  
  return true;
}

void checkLockState() {
  bool currentLockState = digitalRead(LOCK_STATE_PIN);                                    // get current lock state
  if (currentLockState == lastLockState) return;                                          // do nothing if state didn't changed
  Serial.printf("Lock has been %s manually\r\n", currentLockState?"locked":"unlocked");   // print current lock state to serial
  lastLockState = currentLockState;                                                       // update last known lock state
  SinricProLock &myLock = SinricPro[LOCK_ID];                                             // get the LockDevice
  bool success = myLock.sendLockStateEvent(currentLockState);                             // update LockState on Server
  if(!success) {
      Serial.printf("Something went wrong...could not send Event to server!\r\n");
  }
}

void setupWiFi() {
  Serial.printf("\r\n[Wifi]: Connecting");
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  #if defined(ESP8266)
    WiFi.setSleepMode(WIFI_NONE_SLEEP); 
  #elif defined(ESP32)
    WiFi.setSleep(false); 
  #endif

  while (WiFi.status() != WL_CONNECTED) {
    Serial.printf(".");
    delay(250);
  }
  Serial.printf("connected!\r\n[WiFi]: IP-Address is %s\r\n", WiFi.localIP().toString().c_str());
}

void setupSinricPro() {
  SinricProLock &myLock = SinricPro[LOCK_ID];
  myLock.onLockState(onLockState);

  // setup SinricPro
  SinricPro.onConnected([](){ Serial.printf("Connected to SinricPro\r\n"); }); 
  SinricPro.onDisconnected([](){ Serial.printf("Disconnected from SinricPro\r\n"); });
  SinricPro.begin(APP_KEY, APP_SECRET);
}

void setup() {
  Serial.begin(BAUD_RATE); Serial.printf("\r\n\r\n");

  pinMode(LOCK_PIN, OUTPUT);
  pinMode(LOCK_STATE_PIN, INPUT);

  setupWiFi();
  setupSinricPro();
}

void loop() {
  SinricPro.handle();
  checkLockState();
}
