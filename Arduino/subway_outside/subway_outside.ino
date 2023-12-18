#include "WiFiS3.h"
#include <PubSubClient.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <stdio.h>
#include <string.h>
#include <NewPing.h>

// WiFi and MQTT 셋팅
#define WLAN_SSID "class924"
#define WLAN_PASS "kosta90009"
#define MQTT_SERVER "192.168.0.154"
#define MQTT_PORT 1883

// LCD 주소, 행, 렬 선언
#define LCD_ADDRESS 0x27
#define LCD_COLUMNS 16
#define LCD_ROWS 2

LiquidCrystal_I2C lcd(0x27, 16, 2);

// sonar(TrigPin, EchoPin, MaxDistance);
// TrigPin과 EchoPin과 최대제한거리(MaxDistance)의 값을 선언
NewPing sonar[2] = { 
  NewPing(9, 8, 50), 
  NewPing(11, 10, 50), 
};

WiFiClient ethClient;
PubSubClient mqtt_SUB(ethClient);
PubSubClient mqtt_PUB(ethClient);

void callback_SUB(char* topic, byte* payload, unsigned int length);

void setup() {
  Serial.begin(9600);

  // LCD
  lcd.init();
  lcd.clear();
  lcd.backlight();

  // button
  pinMode(6,INPUT);
  pinMode(7,INPUT);

  delay(1000);
  
  // WiFi
  connectWiFi();
  delay(5000);

  // MQTT broker
  mqtt_SUB.setServer(MQTT_SERVER, MQTT_PORT);
  mqtt_PUB.setServer(MQTT_SERVER, MQTT_PORT);
  connectMQTT();
  mqtt_SUB.setCallback(callback_SUB);
}

void loop() {
  if (!mqtt_SUB.connected()) {
    connectMQTT();
  } else if (!mqtt_PUB.connected()) {
    connectMQTT();
  }

  mqtt_SUB.loop();
  mqtt_PUB.loop();
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

  while (!mqtt_SUB.connected()) {
    if (mqtt_SUB.connect("MQ135_Subscribe")) {
      Serial.println("Connected to MQTT pub");
      mqtt_SUB.subscribe("sensor/mq135/#");
    } else {
      Serial.print("Failed to connect to MQTT, rc=");
      Serial.print(mqtt_SUB.state());
      Serial.println(" Retrying in 5 seconds...");
      delay(5000);
    }
  }
  
  while (!mqtt_PUB.connected())
  {
    if (mqtt_PUB.connect("Door_Button_Publish"))
    {
      Serial.println("Connected to MQTT sub");
    }
    else
    {
      Serial.print("Failed to connect to MQTT, rc=");
      Serial.print(mqtt_PUB.state());
      Serial.println(" Retrying in 5 seconds...");
      delay(5000);
    }
  }
}

void callback_SUB(char* topic, byte* payload, unsigned int length) {
  // --------------------MQ135_SUB--------------------
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);
  
  // Counter to keep track of received messages
  int messageCounter;
  if (strcmp(topic, "sensor/mq135/_1") == 0) {
    messageCounter = 1; 
  } else if (strcmp(topic, "sensor/mq135/_2") == 0) {
    messageCounter = 2; 
  } else if (strcmp(topic, "sensor/mq135/_3") == 0) {
    messageCounter = 3; 
  }
  lcd.setCursor(0, 0);
  lcd.print("Station: ");
  lcd.print(messageCounter);

  lcd.setCursor(0, 1);
  lcd.print("Co2: ");
  lcd.print(message);

  // --------------------Door_Button_PUB--------------------
  long sensor1val, sensor2val;
  sensor1val = sonar[0].ping_cm();
  sensor2val = sonar[1].ping_cm();
  
  // 물체에 반사되어돌아온 초음파의 시간을 변수에 저장합니다.     
  Serial.print("A Ping : ");
  // sonar.ping_cm() : 센서 거리를 'cm'로 계산된 값을 출력
  Serial.print(sensor1val);
  Serial.println("cm");
  
  delay(50);          
  Serial.print("B Ping : ");
  Serial.print(sensor2val);
  Serial.println("cm");

  char buffer1[15];
  char buffer2[15];
  char str[] = "cm";

  ltoa(sensor1val, buffer1, 10);
  ltoa(sensor2val, buffer2, 10);
  strcat(buffer1, str);
  strcat(buffer2, str);

  // 측정된 물체로부터 거리값(cm값)을 보여줍니다.
  Serial.println("------------------------"); 
  Serial.println(buffer1); 
  Serial.println(buffer2);
  Serial.println("------------------------"); 

  delay(1000); // 1초마다 측정값을 보여줍니다.

  // button
  int push1=digitalRead(6);
  Serial.print("push1 = ");
  Serial.println(push1);
  
  int push2=digitalRead(7);
  Serial.print("push2 = ");
  Serial.println(push2);
  delay(1000);

  // Publish ultrasonic 센서 값 MQTT 토픽 설정
  mqtt_PUB.publish("sensor/ultrasonic_1", buffer1);
  mqtt_PUB.publish("sensor/ultrasonic_2", buffer2);
  // Publish button 센서 값 MQTT 토픽 설정
  mqtt_PUB.publish("sensor/button_1", String(push1).c_str());
  mqtt_PUB.publish("sensor/button_2", String(push2).c_str());

  lcd.clear();
}