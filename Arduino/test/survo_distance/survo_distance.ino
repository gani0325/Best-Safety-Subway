// Includes the Servo library
#include <Servo.h>.
// Defines Tirg and Echo pins of the Ultrasonic Sensor
#define trigPin 9 // TRIG 핀 설정 (초음파 보내는 핀)
#define echoPin 8 // ECHO 핀 설정 (초음파 받는 핀)

// defining time and distance
long duration;
int distance;
Servo myServo; // Object servo
void setup()
{
    pinMode(trigPin, OUTPUT); // trigPin as an Output
    pinMode(echoPin, INPUT);  // echoPin as an Input
    Serial.begin(9600);
    myServo.attach(12); // Pin Connected To Servo
}
void loop()
{
    // rotating servo i++ depicts increment of one degree
    for (int i = 15; i <= 165; i++)
    {
        myServo.write(i);
        delay(30);
        distance = calculateDistance();

        Serial.print(i);
        Serial.print(",");
        Serial.print(distance);
        Serial.print(".");
    }
    // Repeats the previous lines from 165 to 15 degrees
    for (int i = 165; i > 15; i--)
    {
        myServo.write(i);
        delay(30);
        distance = calculateDistance();
        Serial.print(i);
        Serial.print(",");
        Serial.print(distance);
        Serial.print(".");
    }
    delay(2000);
}
int calculateDistance()
{

    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    // Sets the trigPin on HIGH state for 10 micro seconds
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    duration = pulseIn(echoPin, HIGH);
    distance = duration * 0.034 / 2;
    return distance;
}