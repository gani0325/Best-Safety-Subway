#include "WiFiS3.h"
#include <Ethernet.h>
#include <PubSubClient.h>

// WiFi and MQTT 셋팅
#define WLAN_SSID "class924"
#define WLAN_PASS "kosta90009"
#define MQTT_SERVER "192.168.0.51"
#define MQTT_PORT 1883
#define MQTT_USERNAME "your_mqtt_username"
#define MQTT_KEY "your_mqtt_password"

#define TRIG 9 // TRIG 핀 설정 (초음파 보내는 핀)
#define ECHO 8 // ECHO 핀 설정 (초음파 받는 핀)

///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = WLAN_SSID; // your network SSID (name)
char pass[] = WLAN_PASS; // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;

WiFiClient ethClient;
PubSubClient mqtt(ethClient);

int status = WL_IDLE_STATUS;
WiFiServer server(80);

void setup()
{
  Serial.begin(9600);
  delay(10);

  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
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
  long duration, distance;
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  duration = pulseIn(ECHO, HIGH); // 물체에 반사되어돌아온 초음파의 시간을 변수에 저장합니다.
  // 34000*초음파가 물체로 부터 반사되어 돌아오는시간 /1000000 / 2(왕복값이아니라 편도값이기때문에 나누기2를 해줍니다.)
  // 초음파센서의 거리값이 위 계산값과 동일하게 Cm로 환산되는 계산공식 입니다. 수식이 간단해지도록 적용했습니다.
  distance = duration * 17 / 1000;
  // PC모니터로 초음파 거리값을 확인 하는 코드 입니다.

  // Serial.println(duration ); //초음파가 반사되어 돌아오는 시간을 보여줍니다.
  // Serial.print("\nDistance : ");
  // Serial.print(distance); // 측정된 물체로부터 거리값(cm값)을 보여줍니다.
  // Serial.println(" Cm");

  char buffer2[100];
  char str[] = "cm";
  ltoa(distance, buffer2, 10);
  strcat(buffer2, str);
  Serial.print(buffer2); // 측정된 물체로부터 거리값(cm값)을 보여줍니다.

  delay(1000); // 1초마다 측정값을 보여줍니다.

  // Publish MQ-135센서 값 MQTT 토픽 설정
  mqtt.publish("sensor/mq135", buffer2);

  // 대기를 주기 위한 딜레이
  delay(5000);
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