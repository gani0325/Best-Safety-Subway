#include <MQUnifiedsensor.h>    // 라이버러리 포함
#define placa "Arduino UNO"
#define Voltage_Resolution 5
#define MQ135Pin A0             // A0에 센서 연결
#define type "MQ-135"           // 센서 타입 MQ135
#define ADC_Bit_Resolution 10   // UNO/MEGA/NANO 사용 시 ADC 10비트
#define RatioMQ135CleanAir 3.6  // RS/R0 = 3.6 ppm

MQUnifiedsensor MQ135(placa, Voltage_Resolution, ADC_Bit_Resolution, MQ135Pin, type);  // 센서 선언
unsigned long previousMillis = 0;
const long interval = 500;     // 500밀리초 주기로 동작

void setup() {
  Serial.begin(9600);           // 시리얼 포트 초기화
  MQ135.setRegressionMethod(1);  // _PPM =  a*ratio^b (PPM 농도와 상수 값을 계산하기 위한 수학 모델 설정)
  MQ135.init();
  MQ135.setRL(1);                // RL 값이 1K
  MQ135.setR0(3.12);             // 캘리브레이션하여 이 값을 구함 : MQ135.setR0(calcR0/10);
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    MQ135.update();                         // 데이터를 업데이트하면 arduino가 아날로그 핀의 전압을 읽습니다
    MQ135.setA(110.47); MQ135.setB(-2.862); // CO2 농도를 얻기위한 방정식 값 구성
    float CO2 = MQ135.readSensor();         // 센서는 설정된 모델 및 a 및 b 값을 사용하여 PPM 농도를 읽습니다
    Serial.print("CO2= "); Serial.print(CO2); Serial.println(" ppm");   // 직렬 포트에 CO2 농도를 인쇄합니다.
  }
}