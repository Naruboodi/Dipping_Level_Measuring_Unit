#include <LiquidCrystal_I2C.h>
float x = 16.000;
LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x3F for a 16 chars and 2 line display

void setup() {
  lcd.init();
  lcd.clear();         
  lcd.backlight();      // Make sure backlight is on
}

void loop() {
  // Print a message on both lines of the LCD.
  String str0 = "Measured Values (mm)";
  int midline = (20 - str0.length())/2;
  lcd.setCursor(midline, 0); lcd.print(str0);

  lcd.setCursor(0,1);   //Set cursor to character 2 on line 0
  lcd.print("L1: ");
  lcd.print(int(x));

  String str1 = String("L2: ") + int(x);
  int backline = 20 - str1.length();
  lcd.setCursor(backline,1);   //Set cursor to character 2 on line 0
  lcd.print(str1);

  String str2 = String("L3: ") + int(x);
  int midline2 = (20 - str2.length())/2;
  lcd.setCursor(midline2,2);   //Set cursor to character 2 on line 0
  lcd.print(str2);

  lcd.setCursor(0,3);   //Set cursor to character 2 on line 0
  lcd.print("L5: ");
  lcd.print(int(x));

  String str3 = String("L4: ") + int(x);
  int backline2 = 20 - str3.length();
  lcd.setCursor(backline2,3);   //Set cursor to character 2 on line 0
  lcd.print(str3);
  
  x = random(17-4, 17+4);
  delay(750);
}
