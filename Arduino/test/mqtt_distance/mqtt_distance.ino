#include "WiFiS3.h"
#include <Ethernet.h>
#include <PubSubClient.h>

// WiFi and MQTT 셋팅
#define WLAN_SSID "class924"
#define WLAN_PASS "kosta90009"
#define MQTT_SERVER "192.168.0.154"
#define MQTT_PORT 1883

#define TRIG_1 11 // TRIG 핀 설정 (초음파 보내는 핀)
#define ECHO_1 10 // ECHO 핀 설정 (초음파 받는 핀)

#define TRIG_2 9 // TRIG 핀 설정 (초음파 보내는 핀)
#define ECHO_2 8 // ECHO 핀 설정 (초음파 받는 핀)

///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = WLAN_SSID; // your network SSID (name)
char pass[] = WLAN_PASS; // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;

WiFiClient ethClient;
PubSubClient mqtt(ethClient);

int status = WL_IDLE_STATUS;
WiFiServer server(80);

long duration,sensor1val, sensor2val;
int distance;

void setup()
{
  Serial.begin(9600);
  delay(10);

  pinMode(TRIG_1, OUTPUT);
  pinMode(ECHO_1, INPUT);
  pinMode(TRIG_2, OUTPUT);
  pinMode(ECHO_2, INPUT);

  connectWiFi();
  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE)
  {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true)
      ;
  }

  // WiFi.firmwareVersion() : 모듈에서 실행 중인 펌웨어 버전을 문자열로 반환
  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION)
  {
    Serial.println("Please upgrade the firmware");
  }

  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED)
  {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid); // print the network name (SSID);

    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);
    // wait 10 seconds for connection:
    delay(5000);
  }
  printWifiStatus();
  delay(5000);
  // MQTT broker
  mqtt.setServer(MQTT_SERVER, MQTT_PORT);
  connectMQTT();
}

void loop()
{
  // 물체에 반사되어돌아온 초음파의 시간을 변수에 저장합니다.
  driveSensor(TRIG_1, ECHO_1);
  sensor1val = distance;
  driveSensor(TRIG_2, ECHO_2);
  sensor2val = distance;

  Serial.print("sensor1 = ");
  Serial.print(sensor1val);
  Serial.print(" cm");
  Serial.print("     sensor2 = ");
  Serial.print(sensor2val);
  Serial.println(" cm");  

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

  // Publish ultrasonic 센서 값 MQTT 토픽 설정
  mqtt.publish("sensor/ultrasonic_1", buffer1);
  mqtt.publish("sensor/ultrasonic_2", buffer2);

  // 대기를 주기 위한 딜레이
  delay(3000);
}

void driveSensor(int trigPin,int echoPin){
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);

  // 34000*초음파가 물체로 부터 반사되어 돌아오는시간 /1000000 / 2(왕복값이아니라 편도값이기때문에 나누기2를 해줍니다.)
  // 초음파센서의 거리값이 위 계산값과 동일하게 Cm로 환산되는 계산공식 입니다. 수식이 간단해지도록 적용했습니다.
  distance = duration * 17 / 1000;
}

void connectWiFi()
{
  Serial.println("Connecting to WiFi");
  WiFi.begin(WLAN_SSID, WLAN_PASS);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi");
}

void connectMQTT()
{
  Serial.println("Connecting to MQTT");
  while (!mqtt.connected())
  {
    if (mqtt.connect("MQ135Client"))
    {
      Serial.println("Connected to MQTT");
    }
    else
    {
      Serial.print("Failed to connect to MQTT, rc=");
      Serial.print(mqtt.state());
      Serial.println(" Retrying in 5 seconds...");
      delay(5000);
    }
  }
}

void printWifiStatus()
{
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  // WiFi.localIP() : WiFi 쉴드의 IP 주소
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
  // print where to go in a browser:
  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);
}