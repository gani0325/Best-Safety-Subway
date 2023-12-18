#include <MQUnifiedsensor.h>
#include "WiFiS3.h"
#include <Ethernet.h>
#include <PubSubClient.h>
#include <Arduino_FreeRTOS.h>

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

int status = WL_IDLE_STATUS;
WiFiServer server(80);

MQUnifiedsensor MQ135_1("Arduino UNO", 5, 10, A0, "MQ-135_1");
MQUnifiedsensor MQ135_2("Arduino UNO", 5, 10, A1, "MQ-135_2");

unsigned long previousMillis = 0;
const long interval = 500;

TaskHandle_t Handle_aTask;
TaskHandle_t Handle_bTask;

static void taskMQ135_1(void* pvParameters) {
    MQ135_1.setRegressionMethod(1);
    MQ135_1.init();
    MQ135_1.setRL(1);
    MQ135_1.setR0(3.12);

    while (1) {
        MQ135_1.update();
        MQ135_1.setA(110.47);
        MQ135_1.setB(-2.862);
        float CO2_1 = MQ135_1.readSensor();

        Serial.print("CO2_1= ");
        Serial.println(CO2_1);

        mqtt.publish("sensor/mq135_1", String(CO2_1).c_str());

        vTaskDelay( 200 / portTICK_PERIOD_MS ); 
        delay(2000);
    }
}

static void taskMQ135_2(void* pvParameters) {
    MQ135_2.setRegressionMethod(1);
    MQ135_2.init();
    MQ135_2.setRL(1);
    MQ135_2.setR0(3.12);

    while (1) {
        MQ135_2.update();
        MQ135_2.setA(110.47);
        MQ135_2.setB(-2.862);
        float CO2_2 = MQ135_2.readSensor();

        Serial.print("CO2_2= ");
        Serial.println(CO2_2);

        mqtt.publish("sensor/mq135_2", String(CO2_2).c_str());
        delay(2000);
    }
    Serial.println("Thread B: Deleting");
    vTaskDelete(NULL);
}

void setup() {

    Serial.begin(9600);

    delay(1000); // prevents usb driver crash on startup, do not omit this
    while(!Serial);  // Wait for Serial terminal to open port before starting program
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

    Serial.println("");
    Serial.println("******************************");
    Serial.println("        Program start         ");
    Serial.println("******************************");

    // Create the threads that will be managed by the rtos
    // Sets the stack size and priority of each task
    // Also initializes a handler pointer to each task, which are important to communicate with and retrieve info from tasks
    xTaskCreate(taskMQ135_1,"taskMQ135_1",256, NULL, tskIDLE_PRIORITY + 2, &Handle_aTask);
    xTaskCreate(taskMQ135_2,"taskMQ135_2",256, NULL, tskIDLE_PRIORITY + 1, &Handle_bTask);

    // Start the RTOS, this function will never return and will schedule the tasks.
    vTaskStartScheduler();
}

void loop() {
    // NOTHING
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