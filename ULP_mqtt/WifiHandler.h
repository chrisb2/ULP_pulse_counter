// WiFiHandler.h
#ifndef WIFIHANDLER_H
#define WIFIHANDLER_H
#include <WiFi.h>
#include <AsyncMqttClient.h>
#include <ArduinoJson.h>
#include <set>

#include <Arduino.h> //for serial

namespace WiFiHandler {
    bool setupWiFi();
    bool setupMQTT();
    void sendData(uint32_t count, float rate);
    void disconnectMQTT();
    void disconnectWiFi();
}

#endif
