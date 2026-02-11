#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ArduinoJson.h>
#include <time.h>

// WiFi Configuration
const char* ssid = "RCA";
const char* password = "@RcaNyabihu2023";
const uint32_t WIFI_TIMEOUT_MS = 30000;

// MQTT Configuration
const char* mqtt_server = "broker.emqx.io";
const char* team_id = "iot_shield_2026";
const uint16_t MQTT_PORT = 1883;

// Topics
const String topic_status = "rfid/" + String(team_id) + "/card/status";
const String topic_balance = "rfid/" + String(team_id) + "/card/balance";
const String topic_topup = "rfid/" + String(team_id) + "/card/topup";
const String topic_health = "rfid/" + String(team_id) + "/device/health";
const String topic_lwt = "rfid/" + String(team_id) + "/device/status";

// Pin Mapping
#define RST_PIN         D3          
#define SS_PIN          D4          

MFRC522 mfrc522(SS_PIN, RST_PIN);
WiFiClient espClient;
PubSubClient client(espClient);

unsigned long last_health_report = 0;
const unsigned long HEALTH_INTERVAL = 60000; // 60 seconds

void sync_time() {
  Serial.print("Syncing time with NTP...");
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("\nTime synchronized");
}

unsigned long get_unix_time() {
  time_t now = time(nullptr);
  return (unsigned long)now;
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  unsigned long start_attempt_ms = millis();
  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - start_attempt_ms > WIFI_TIMEOUT_MS) {
      Serial.println("\nWiFi connection timeout! Restarting...");
      ESP.restart();
    }
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  
  StaticJsonDocument<256> doc;
  deserializeJson(doc, payload, length);
  
  const char* uid = doc["uid"];
  float amount = doc["amount"];

  // Simulation of business logic
  float simulatedOldBalance = 50.0; 
  float newBalance = simulatedOldBalance + amount;

  StaticJsonDocument<200> responseDoc;
  responseDoc["uid"] = uid;
  responseDoc["new_balance"] = newBalance;
  responseDoc["status"] = "success";
  responseDoc["ts"] = get_unix_time();
  
  char buffer[200];
  serializeJson(responseDoc, buffer);
  client.publish(topic_balance.c_str(), buffer);
  
  Serial.print("Updated balance for ");
  Serial.print(uid);
  Serial.print(": ");
  Serial.println(newBalance);
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    
    // Set LWT (Last Will and Testament)
    String clientId = "ESP8266_Shield_" + String(ESP.getChipId(), HEX);
    
    if (client.connect(clientId.c_str(), topic_lwt.c_str(), 1, true, "offline")) {
      Serial.println("connected");
      
      // Publish online status
      client.publish(topic_lwt.c_str(), "online", true);
      
      client.subscribe(topic_topup.c_str());
      client.subscribe(topic_health.c_str());
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void publish_health() {
  StaticJsonDocument<256> doc;
  doc["status"] = "online";
  doc["ip"] = WiFi.localIP().toString();
  doc["rssi"] = WiFi.RSSI();
  doc["free_heap"] = ESP.getFreeHeap();
  doc["ts"] = get_unix_time();

  char buffer[256];
  serializeJson(doc, buffer);
  client.publish(topic_health.c_str(), buffer);
  Serial.println("Health report published");
}

void setup() {
  Serial.begin(115200);
  SPI.begin();
  mfrc522.PCD_Init();
  
  setup_wifi();
  sync_time();
  
  client.setServer(mqtt_server, MQTT_PORT);
  client.setCallback(callback);
  
  Serial.println("✓ System initialized successfully");
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    setup_wifi();
  }

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Periodic health report
  unsigned long now = millis();
  if (now - last_health_report > HEALTH_INTERVAL) {
    last_health_report = now;
    publish_health();
  }

  // RFID Scanning logic
  if (mfrc522.PCD_IsNewCardPresent() && mfrc522.PCD_ReadUID()) {
    String uid = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      uid += String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
      uid += String(mfrc522.uid.uidByte[i], HEX);
    }
    uid.toUpperCase();
    
    float currentBalance = 50.0; // Simulated read

    Serial.print("Card Detected: ");
    Serial.print(uid);
    Serial.print(" | Balance: ");
    Serial.println(currentBalance);

    StaticJsonDocument<255> doc;
    doc["uid"] = uid;
    doc["balance"] = currentBalance;
    doc["status"] = "detected";
    doc["ts"] = get_unix_time();
    
    char buffer[255];
    serializeJson(doc, buffer);
    client.publish(topic_status.c_str(), buffer);

    mfrc522.PCD_StopCrypto1();
    delay(2000); // Debounce
  }
}
