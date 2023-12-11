//sub.ino
#include "WiFiS3.h"
#include <PubSubClient.h>
// LCD를 사용하기 위해 라이브러리를 추가합니다.
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

#define WLAN_SSID "class924"
#define WLAN_PASS "kosta90009"
#define MQTT_SERVER "192.168.0.51"
#define MQTT_PORT 1883
#define MQTT_USERNAME "your_mqtt_username"
#define MQTT_KEY "your_mqtt_password"


#define LCD_ADDRESS 0x27
#define LCD_COLUMNS 16
#define LCD_ROWS 2
// 16x2 LCD객체를 생성합니다. 이때 확인한 I2C의 주소값을 넣어줍니다.
LiquidCrystal_I2C lcd(0x27, 16, 2);

WiFiClient ethClient;
PubSubClient mqtt(ethClient);
void callback(char* topic, byte* payload, unsigned int length);

void setup() {
  Serial.begin(9600);
  delay(10);

  connectWiFi();

  mqtt.setServer(MQTT_SERVER, MQTT_PORT);
  connectMQTT();
  delay(2000);
  mqtt.setCallback(callback);
  //Display 초기화
  // lcd.begin(LCD_COLUMNS, LCD_ROWS);
      lcd.init();
    lcd.backlight();
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
    if (mqtt.connect("LCDClient")) {
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
  // 출력 확인
  Serial.print(message);
  
  lcd.setCursor(0, 0);
  lcd.print("Distance: ");
  lcd.print(message);
  delay(2000);
  lcd.clear();
}