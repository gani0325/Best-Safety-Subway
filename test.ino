#include <Wire.h>
#include <Ethernet.h>
#include <PubSubClient.h>

// WiFi and MQTT 셋팅
#define WLAN_SSID       "your_wifi_ssid"
#define WLAN_PASS       "your_wifi_password"
#define MQTT_SERVER     "mqtt_broker_ip"
#define MQTT_PORT       1883
#define MQTT_USERNAME   "your_mqtt_username"
#define MQTT_KEY        "your_mqtt_password"

// MQ-135 센서 핀
#define MQ135_PIN A0

EthernetClient ethClient;
PubSubClient mqtt(ethClient);

void setup() {
  Serial.begin(115200);
  delay(10);

  // 와이파이 연결
  connectWiFi();

  // MQTT broker
  mqtt.setServer(MQTT_SERVER, MQTT_PORT);
  connectMQTT();
}

void loop() {
  // MQ-135 센서 값 읽음
  int mq135Value = analogRead(MQ135_PIN);

  // Publish MQ-135센서 값 MQTT 토픽 설정
  mqtt.publish("sensor/mq135", String(mq135Value).c_str());

  // 대기를 주기 위한 딜레이
  delay(5000);
}

void connectWiFi() {
  Serial.println("Connecting to WiFi");
  WiFi.begin(WLAN_SSID, WLAN_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi");
}

void connectMQTT() {
  Serial.println("Connecting to MQTT");
  while (!mqtt.connected()) {
    if (mqtt.connect("MQ135Client", MQTT_USERNAME, MQTT_KEY)) {
      Serial.println("Connected to MQTT");
    } else {
      Serial.print("Failed to connect to MQTT, rc=");
      Serial.print(mqtt.state());
      Serial.println(" Retrying in 5 seconds...");
      delay(5000);
    }
  }
}
