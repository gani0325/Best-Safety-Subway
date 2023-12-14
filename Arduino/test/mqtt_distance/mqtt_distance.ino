#include "WiFiS3.h"
#include <Ethernet.h>
#include <PubSubClient.h>
#include <NewPing.h>

// WiFi and MQTT 셋팅
#define WLAN_SSID "class924"
#define WLAN_PASS "kosta90009"
#define MQTT_SERVER "192.168.0.154"
#define MQTT_PORT 1883

// sonar(TrigPin, EchoPin, MaxDistance);
// TrigPin과 EchoPin과 최대제한거리(MaxDistance)의 값을 선언
NewPing sonar[2] = { 
  NewPing(9, 8, 50), 
  NewPing(11, 10, 50), 
};

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

  // Publish ultrasonic 센서 값 MQTT 토픽 설정
  mqtt.publish("sensor/ultrasonic_1", buffer1);
  mqtt.publish("sensor/ultrasonic_2", buffer2);

  // 대기를 주기 위한 딜레이
  delay(3000);
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