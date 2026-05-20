
// WiFiHandler.cpp
#include "WiFiHandler.h"
#include "Configuration.h"

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

bool WiFiHandler::setupWiFi() {
  long start = millis();

  // Connect to Wi-Fi
  Serial.printf("\nConnecting to WiFi: %s\n", WIFI_SSID);

  IPAddress ip(192,168,1,14), gw(192,168,1,1), sn(255,255,255,0), dns(8,8,8,8);
  WiFi.config(ip, gw, sn, dns);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  WiFi.setTxPower(WIFI_POWER_15dBm);

  while (WiFi.status() != WL_CONNECTED && (millis() - start) < 10000) {
    yield();
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("WiFi connect (%s, %s) in %lu ms\n", 
      WiFi.localIP().toString().c_str(), WiFi.macAddress().c_str(), millis() - start);
    return true;
  }
  else {
    return false;
  }
  
    
}

bool WiFiHandler::setupMQTT() {  
  long start = millis();

  // Set MQTT server
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  // Loop until we're reconnected
  while (!mqttClient.connected() && (millis() - start) < 1000) {
    Serial.print("Attempting MQTT connection...");
    // Try to connect
    String deviceName = String(DEVICE_NAME) + String(random(1000, 9999));
    if (mqttClient.connect(deviceName.c_str(), MQTT_USER, MQTT_PASSWORD)) {
      Serial.printf("MQTT connect in %lu ms\n", millis() - start);
      return true;
    } else {
      Serial.printf("Failed, rc=%i\n", mqttClient.state());
    }
  }
  return false;   
}

float readBatteryVoltage() {
    int rawADC = analogRead(BATTERY_PIN);
    Serial.printf("RAW battery ADC: %i\n", rawADC);
    float voltage = (rawADC / float(ADC_RESOLUTION)) * REF_VOLTAGE * VOLTAGE_DIVIDER;
    return voltage;
}


void WiFiHandler::sendData(uint32_t count, float rate) {
  char countTopic[50], rateTopic[50], signalTopic[50], voltageTopic[50];
  snprintf(countTopic, sizeof(countTopic), "%s/%s/count", TOPIC_PREFIX, DEVICE_NAME);
  snprintf(rateTopic, sizeof(rateTopic), "%s/%s/rate", TOPIC_PREFIX, DEVICE_NAME);
  snprintf(signalTopic, sizeof(signalTopic), "%s/%s/signal", TOPIC_PREFIX, DEVICE_NAME);
  snprintf(voltageTopic, sizeof(voltageTopic), "%s/%s/voltage", TOPIC_PREFIX, DEVICE_NAME);


  mqttClient.publish(countTopic, String(count).c_str(), true);
  mqttClient.loop();
  mqttClient.publish(rateTopic, String(rate).c_str(), true);
  mqttClient.loop();

  // Get WiFi signal strength
  int32_t signalStrength = WiFi.RSSI();
  mqttClient.publish(signalTopic, String(signalStrength).c_str(), true);
  mqttClient.loop();

  // Send battery voltage
  float batteryVoltage = readBatteryVoltage();
  mqttClient.publish(voltageTopic, String(batteryVoltage).c_str(), true);
  mqttClient.loop();

  Serial.println(countTopic);
  Serial.println(rateTopic);
  Serial.println(signalTopic);
  Serial.println(voltageTopic);
  Serial.printf("Data sent: count=%u, rate=%.2f, signal=%d dBm, voltage=%.2f V\n", count, rate, signalStrength, batteryVoltage);
}

void WiFiHandler::disconnectMQTT() {
  mqttClient.loop();
  mqttClient.disconnect();
  delay(100);
  mqttClient.loop();
  delay(100);
}

void WiFiHandler::disconnectWiFi() {
  WiFi.disconnect(true); 
}
