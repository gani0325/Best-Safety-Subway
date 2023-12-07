#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// lcd 객체 선언
LiquidCrystal_I2C lcd(0x27, 16, 2); // 주소, 열, 행

void setup()
{
    Serial.begin(9600);
    lcd.init();
    lcd.backlight();
}

void loop()
{
    
    lcd.setCursor(0, 0);
    lcd.print("hedddllo");
    lcd.setCursor(0, 1);
    lcd.print("gani");
    lcd.clear();
}
