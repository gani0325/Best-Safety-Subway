#include <Servo.h>

Servo servo;

void setup()
{
    servo.attach(10);
}

void loop()
{
    for (int angle = 0; angle < 180; angle++)
    {
        servo.write(angle);
        delay(1000);
    }
}