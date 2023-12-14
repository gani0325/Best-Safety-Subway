#include <MQUnifiedsensor.h>
#include "WiFiS3.h"
#include <Ethernet.h>
#include <PubSubClient.h>

// WiFi and MQTT settings
#define WLAN_SSID "class924"
#define WLAN_PASS "kosta90009"
#define MQTT_SERVER "192.168.0.154"
#define MQTT_PORT 1883

#define MQ135_PIN_1 A0
#define MQ135_PIN_2 A1

char ssid[] = WLAN_SSID;
char pass[] = WLAN_PASS;
int keyIndex = 0;

WiFiClient ethClient;
PubSubClient mqtt(ethClient);

MQUnifiedsensor MQ135_1("Arduino UNO", 5, 10, A0, "MQ-135_1");
MQUnifiedsensor MQ135_2("Arduino UNO", 5, 10, A1, "MQ-135_2");

unsigned long previousMillis = 0;
const long interval = 500;

void setup() {
  Serial.begin(9600);
  delay(10);

  pinMode(MQ135_PIN_1, INPUT);
  pinMode(MQ135_PIN_2, INPUT);
  connectWiFi();

  MQ135_1.setRegressionMethod(1);
  MQ135_1.init();
  MQ135_1.setRL(1);
  MQ135_1.setR0(3.12);

  MQ135_2.setRegressionMethod(1);
  MQ135_2.init();
  MQ135_2.setRL(1);
  MQ135_2.setR0(3.12);

  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    while (true)
      ;
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);
    WiFi.begin(ssid, pass);
    delay(5000);
  }

  printWifiStatus();
  delay(5000);

  mqtt.setServer(MQTT_SERVER, MQTT_PORT);
  connectMQTT();
}

void loop() {
  MQ135_1.update();
  MQ135_1.setA(110.47);
  MQ135_1.setB(-2.862);
  float CO2_1 = MQ135_1.readSensor();

  Serial.print("CO2_1= ");
  Serial.println(CO2_1);

  mqtt.publish("sensor/mq135_1", String(CO2_1).c_str());

  // Delay for a short interval to avoid flooding the MQTT broker
  delay(100);

  MQ135_2.update();
  MQ135_2.setA(110.47);
  MQ135_2.setB(-2.862);
  float CO2_2 = MQ135_2.readSensor();

  Serial.print("CO2_2= ");
  Serial.println(CO2_2);

  mqtt.publish("sensor/mq135_2", String(CO2_2).c_str());

  // Delay for a longer interval before the next iteration
  delay(interval);
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
    if (mqtt.connect("MQ135Client")) {
      Serial.println("Connected to MQTT");
    } else {
      Serial.print("Failed to connect to MQTT, rc=");
      Serial.print(mqtt.state());
      Serial.println(" Retrying in 5 seconds...");
      delay(5000);
    }
  }
}

void printWifiStatus() {
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");

  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);
}
