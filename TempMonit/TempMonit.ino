#include <SoftwareSerial.h>

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

#define AnalogRead 2000 //период чтения напряжения, ms
#define DrawTime 1000 //период обновления экрана, ms
#define KeyReadTime 20 //период проверки нажатия клавиатуры

LiquidCrystal_I2C lcd(0x27,16,2);  // Устанавливаем дисплей
SoftwareSerial SIM800(2, 3); //подгтавливаем программный serial порт на пинах 2 и 3

volatile int tyme = 0; //переменная прерываний
bool dt = false; //флаг обновления экрана
bool rt = false; //флаг чтения температуры
bool kr = 0; //флаг периода проверки клавиатуры

void setup() {
  OCR0A = 0xAF; //прерывание
  TIMSK0 |= _BV(OCIE0A); //прерывание

  Serial.begin(9600);
  SIM800.begin(9600); //запускаем программный serial порт
  
  lcd.init(); // initialize the LCD
  lcd.backlight(); // Включаем подсветку дисплея

  Serial.println("Start!");
  SIM800.println("AT"); //Готовm модул к работе?
}

SIGNAL(TIMER0_COMPA_vect) { //прерывание считывающие мощность
  tyme = tyme + 1;
  if (tyme % AnalogRead == 0) { rt = 1; };
  if (tyme % DrawTime == 0) { dt = 1; };
  if (tyme % KeyReadTime == 0) { kr = 1; };
}

void DrawMenu () {
//  Serial.println("DrawMenu");
//  lcd.noBacklight();
//  lcd.setCursor(3,0);
//  lcd.print("Hello, world!");
//  lcd.setCursor(2,1);
//  lcd.print("Ywrobot Arduino!"); 
}

void KeyPad () {
  
}

void ReadAnalog () {
   
}
void loop() {
  if (rt == 1) { rt = 0; ReadAnalog(); };  
  if (dt == 1) { dt = 0; DrawMenu(); };
  if (kr == 1) { kr = 0; KeyPad(); };

  if (SIM800.available())           // Ожидаем прихода данных (ответа) от модема...
    Serial.write(SIM800.read());    // ...и выводим их в Serial
  if (Serial.available())           // Ожидаем команды по Serial...
    SIM800.write(Serial.read());    // ...и отправляем полученную команду модему 
}



