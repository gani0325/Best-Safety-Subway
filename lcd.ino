#include <Wire.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <LiquidCrystal_I2C.h>


#define WLAN_SSID       "your_wifi_ssid"
#define WLAN_PASS       "your_wifi_password"
#define MQTT_SERVER     "mqtt_broker_ip"
#define MQTT_PORT       1883
#define MQTT_USERNAME   "your_mqtt_username"
#define MQTT_KEY        "your_mqtt_password"


#define LCD_ADDRESS     0x27
#define LCD_COLUMNS     16
#define LCD_ROWS        2
LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLUMNS, LCD_ROWS);

EthernetClient ethClient;
PubSubClient mqtt(ethClient);

void setup() {
  Serial.begin(115200);
  delay(10);


  connectWiFi();


  mqtt.setServer(MQTT_SERVER, MQTT_PORT);
  connectMQTT();

 
  lcd.begin(LCD_COLUMNS, LCD_ROWS);
}

void loop() {
 
  if (!mqtt.connected()) {
    connectMQTT();
  }


  mqtt.loop();

  delay(1000);
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
    if (mqtt.connect("LCDClient", MQTT_USERNAME, MQTT_KEY)) {
      Serial.println("Connected to MQTT");
  
      mqtt.subscribe("sensor/mq135");
    } else {
      Serial.print("Failed to connect to MQTT, rc=");
      Serial.print(mqtt.state());
      Serial.println(" Retrying in 5 seconds...");
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
 
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  lcd.clear();
  lcd.print("MQ-135: ");
  lcd.print(message);
}
