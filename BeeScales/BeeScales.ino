#include <HX711.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

#define ScaleRead 500 //период чтения  чтения данных с тензодатчиков, ms
#define DrawTime 1000 //период обновления экрана, ms
#define KeyReadTime 20 //период проверки нажатия клавиатуры

#define LcdPowerPin 12 //пин питания экрана

HX711 scale;  
LiquidCrystal_I2C lcd(0x3F,16,2);  // Устанавливаем дисплей
SoftwareSerial SIM800(2, 3); //подгтавливаем программный serial порт на пинах 2 и 3


volatile int tyme = 0; //переменная прерываний
bool dt = false; //флаг обновления экрана
bool scR = false; //флаг чтения данных с тензодатчиков
bool kr = 0; //флаг периода проверки клавиатуры

float scaleValue = 0;
float sumScaleValue = 0;
byte cntScale = 0;

//---------------- ФУНКЦИИ ----------------
void devicePowerUP() {
  digitalWrite(LcdPowerPin, HIGH);
}

void devicePowerDOWN() {
  digitalWrite(LcdPowerPin, LOW);
}

//декодирует аналоговый сигнал от клавиатуры в номер нажатой кнопки
byte key() {                       
    //1(SEL-721), 2(LEFT-480),3(DOWN-306),4(UP-131),5(RIGHT-0)
//    int val = analogRead(KEYS);
//    if (val < 50) return RIGHT; 
//    if (val < 150) return UP;
//    if (val < 350) return DOWN;
//    if (val < 500) return LEFT;
//    if (val < 800) return SELECT;
//    return 0;
}

void drawNumber (float number, byte precision) {
  int intNumber = 0; //Целая часть
  byte fraction = 0; //Дробная часть
  bool mines = false;
       
  precision = pow(10, precision);
  number = round(number*precision);
  
  if (number < 0) 
    mines = true; 
  intNumber = (int) number / precision;
  fraction = (byte) number % precision;

  lcd.print(intNumber);
  lcd.print(".");
  lcd.print(fraction);
  if (fraction == 0) {
    for (int i = 1; i<(precision/10); i=i*10) {
      lcd.print(0);
    }
  }
}

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

  return digits[2];
}


//---------------- System ФУНКЦИИ ----------------
void DrawMenu () {
  lcd.setCursor(0, 0);
  lcd.print("                "); //очистим ранее выведенное
  lcd.setCursor(0, 0);
  
  drawNumber(scaleValue, 2);
  lcd.print(" kg");
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

  pinMode(LcdPowerPin, OUTPUT);
  digitalWrite(LcdPowerPin, HIGH);
  
  // HX711.DOUT  - pin #A1
  // HX711.PD_SCK - pin #A0
  scale.begin(A1, A0);
  scale.set_scale(24960);     // this value is obtained by calibrating the scale with known weights; see the README for details
  scale.tare();               // reset the scale to 0

  lcd.init(); // initialize the LCD
  lcd.backlight(); // Включаем подсветку дисплея
   
  Serial.begin(9600);
  SIM800.begin(9600); //запускаем программный serial порт

  Serial.println("Start!");
  SIM800.println("AT"); //Готовm модул к работе?
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



