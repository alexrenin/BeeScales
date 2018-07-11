#include <SoftwareSerial.h>
#include <HX711.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

#define ScaleRead 500 //период чтения  чтения данных с тензодатчиков, ms
#define DrawTime 1000 //период обновления экрана, ms
#define KeyReadTime 20 //период проверки нажатия клавиатуры

LiquidCrystal_I2C lcd(0x27,16,2);  // Устанавливаем дисплей
SoftwareSerial SIM800(2, 3); //подгтавливаем программный serial порт на пинах 2 и 3
HX711 scale;  

volatile int tyme = 0; //переменная прерываний
bool dt = false; //флаг обновления экрана
bool scR = false; //флаг чтения данных с тензодатчиков
bool kr = 0; //флаг периода проверки клавиатуры

float scaleValue = 0;

//---------------- ФУНКЦИИ ----------------
float GetMedian (float digits[5]) {
  byte samples = 5;
  float temp = 0;
  for (int i = 0; i < samples; i++){
    for (int j = 0; j < samples - 1; j++){
      if (digits[j] > digits[j + 1]){
        temp = digits[j];
        digits[j] = digits[j + 1];
        digits[j + 1] = temp;
      }
    }
  }

  for (int i = 0; i < samples; i++) {
    Serial.print("  ");
    Serial.println(digits[i]);
  }

  return digits[2];
}


//---------------- System ФУНКЦИИ ----------------
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

void ReadScale () {
  float readDigits[5] = {0, 0, 0, 0, 0};
  
  for (byte i = 0; i<5; i++) {
    readDigits[i] = scale.get_units();
  }

  scaleValue = GetMedian (readDigits);
  Serial.println(scaleValue);

}


void setup() {
  OCR0A = 0xAF; //прерывание
  TIMSK0 |= _BV(OCIE0A); //прерывание

  Serial.begin(9600);
  SIM800.begin(9600); //запускаем программный serial порт
  
  lcd.init(); // initialize the LCD
  lcd.backlight(); // Включаем подсветку дисплея

  Serial.println("Start!");
  SIM800.println("AT"); //Готовm модул к работе?

  // HX711.DOUT  - pin #A1
  // HX711.PD_SCK - pin #A0
  scale.begin(A1, A0);
  scale.set_scale(24960);     // this value is obtained by calibrating the scale with known weights; see the README for details
  scale.tare();               // reset the scale to 0
}

//прерывание, формирует мини операционную систему (system)
SIGNAL(TIMER0_COMPA_vect) { 
  tyme = tyme + 1;
  if (tyme % ScaleRead == 0) { scR = 1; };
  if (tyme % DrawTime == 0) { dt = 1; };
  if (tyme % KeyReadTime == 0) { kr = 1; };
}

void loop() {
  if (scR == 1) { scR = 0; ReadScale(); };  
  if (dt == 1) { dt = 0; DrawMenu(); };
  if (kr == 1) { kr = 0; KeyPad(); };

  if (SIM800.available())           // Ожидаем прихода данных (ответа) от модема...
    Serial.write(SIM800.read());    // ...и выводим их в Serial
  if (Serial.available())           // Ожидаем команды по Serial...
    SIM800.write(Serial.read());    // ...и отправляем полученную команду модему 
}



