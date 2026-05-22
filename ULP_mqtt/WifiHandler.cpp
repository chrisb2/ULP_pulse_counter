// WiFiHandler.cpp
#include "WiFiHandler.h"
#include "Configuration.h"

AsyncMqttClient mqttClient;
static volatile bool mqttConnected = false;

// Track unacknowledged publish packet IDs
static std::set<uint16_t> pendingPacketIds;

// ── Callbacks ────────────────────────────────────────────────────────────────

static void onMqttConnect(bool sessionPresent) {
  mqttConnected = true;
}

static void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  mqttConnected = false;
  Serial.printf("MQTT disconnected, reason: %d\n", static_cast<int>(reason));
}

static void onMqttPublish(uint16_t packetId) {
  pendingPacketIds.erase(packetId);
  Serial.printf("ACK received for packetId: %u (%u remaining)\n", 
    packetId, pendingPacketIds.size());
}

// ── WiFi ─────────────────────────────────────────────────────────────────────

bool WiFiHandler::setupWiFi() {
  long start = millis();

  Serial.printf("Connecting to WiFi: %s\n", WIFI_SSID);

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
  return false;
}

void WiFiHandler::disconnectWiFi() {
  WiFi.disconnect(true);
}

// ── MQTT ─────────────────────────────────────────────────────────────────────

bool WiFiHandler::setupMQTT() {
  long start = millis();

  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onPublish(onMqttPublish);

  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  mqttClient.setCredentials(MQTT_USER, MQTT_PASSWORD);

  String deviceName = String(DEVICE_NAME) + String(random(1000, 9999));
  mqttClient.setClientId(deviceName.c_str());

  Serial.print("MQTT connect");
  mqttClient.connect();

  // Wait for connection callback to fire
  while (!mqttConnected && (millis() - start) < 3000) {
    yield();
  }

  if (mqttConnected) {
    Serial.printf(" in %lu ms\n", millis() - start);
    return true;
  }

  Serial.println("MQTT connection timed out");
  return false;
}

void WiFiHandler::disconnectMQTT() {
  // Wait for all QoS-1 ACKs before disconnecting
  long start = millis();
  while (!pendingPacketIds.empty() && (millis() - start) < 3000) {
    yield();
  }

  if (!pendingPacketIds.empty()) {
    Serial.printf("Disconnecting with %u unacknowledged packets\n", 
      pendingPacketIds.size());
  }

  mqttClient.disconnect();
  // Allow disconnect to propagate before sleeping/cutting power
  while (mqttConnected && (millis() - start) < 500) {
    yield();
  }
  Serial.printf("MQTT disconnected in %dms: %d\n", (millis() - start), !mqttConnected);
}

// ── Helpers ───────────────────────────────────────────────────────────────────

static float readBatteryVoltage() {
  int rawADC = analogRead(BATTERY_PIN);
  return (rawADC / float(ADC_RESOLUTION)) * REF_VOLTAGE * VOLTAGE_DIVIDER;
}

static void publish(const char *topic, const char *payload) {
  // QoS 1, retain = true  →  matches the original retained publish
  uint16_t packetId = mqttClient.publish(topic, 1, true, payload);
  if (packetId > 0) {
    pendingPacketIds.insert(packetId);
  }
  Serial.printf("Publish %s:%s \tpacketId:%u\n", topic, payload, packetId);
}

// ── Data sending ──────────────────────────────────────────────────────────────

void WiFiHandler::sendData(uint32_t count, float rate) {
  char countTopic[50], rateTopic[50], signalTopic[50], voltageTopic[50];
  snprintf(countTopic,   sizeof(countTopic),   "%s/%s/count",   TOPIC_PREFIX, DEVICE_NAME);
  snprintf(rateTopic,    sizeof(rateTopic),    "%s/%s/rate",    TOPIC_PREFIX, DEVICE_NAME);
  snprintf(signalTopic,  sizeof(signalTopic),  "%s/%s/signal",  TOPIC_PREFIX, DEVICE_NAME);
  snprintf(voltageTopic, sizeof(voltageTopic), "%s/%s/voltage", TOPIC_PREFIX, DEVICE_NAME);

  int32_t signalStrength = WiFi.RSSI();
  float batteryVoltage = readBatteryVoltage();

  publish(countTopic,   String(count).c_str());
  publish(rateTopic,    String(rate).c_str());
  publish(signalTopic,  String(signalStrength).c_str());
  publish(voltageTopic, String(batteryVoltage).c_str());

  Serial.printf("Data sent: count=%u, rate=%.2f, signal=%d dBm, voltage=%.2f V\n",
    count, rate, signalStrength, batteryVoltage);
}