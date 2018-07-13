#include <HX711.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

#define ScaleRead 150 //период чтения  чтения данных с тензодатчиков, ms
#define DrawTime 1000 //период обновления экрана, ms
#define KeyReadTime 20 //период проверки нажатия клавиатуры, ms

#define KeypadPin A2 //За сколько измерений усреднять значения веса

#define CntAvScaleRead 5 //За сколько измерений усреднять значения веса

#define SELECT 1 //коды клавиш
#define UP 2 //коды клавиш
#define DOWN 3 //коды клавиш
#define LEFT 4 //коды клавиш
#define RIGHT 5 //коды клавиш


HX711 scale;  
LiquidCrystal_I2C lcd(0x3F,16,2);  // Устанавливаем дисплей

volatile int tyme = 0; //переменная прерываний
bool dt = false; //флаг обновления экрана
bool scR = false; //флаг чтения данных с тензодатчиков
bool kr = 0; //флаг периода проверки клавиатуры

//scale variable
float readDigits[5] = {0, 0, 0, 0, 0};
byte cntReadDigits = 0;
float sumScaleValue = 0;
byte cntSumScale = 0;
float scaleValue = 0;

//keypad variable
byte curPresKey = 0; //код текущей нажатой кнопки

//---------------- ФУНКЦИИ ----------------

//декодирует аналоговый сигнал от клавиатуры в номер нажатой кнопки
byte key() {                       
    //1(SEL-741), 2(LEFT-0),3(DOWN-327),4(UP-143),5(RIGHT-503)
    int val = analogRead(KeypadPin);
//    Serial.println(val);
    if (val < 50) return LEFT; 
    if (val < 200) return UP;
    if (val < 400) return DOWN;
    if (val < 600) return RIGHT;
    if (val < 800) return SELECT;
    return 0;
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
  lcd.setCursor(0, 1);
  lcd.print("                "); //очистим ранее выведенное
  lcd.setCursor(0, 1);
  
  drawNumber(scaleValue, 2);
  lcd.print(" kg");
}

void KeyPad () {
 byte keyCode = key(); 
 Serial.println(keyCode);
}

void ReadScale () {
  readDigits[cntReadDigits] = scale.get_units();
  cntReadDigits++;
  
  if (cntReadDigits > 4) {
    sumScaleValue += GetMedian (readDigits);
    cntSumScale++;
    cntReadDigits = 0;

    if (cntSumScale >= CntAvScaleRead) {
      scaleValue = sumScaleValue/cntSumScale;
      //  Serial.println(scaleValue);
      cntSumScale = 0;
      sumScaleValue = 0;
    }
  }
}


void setup() {
  OCR0A = 0xAF; //прерывание
  TIMSK0 |= _BV(OCIE0A); //прерывание

  // HX711.DOUT  - pin #A1
  // HX711.PD_SCK - pin #A0
  scale.begin(A1, A0);
  scale.set_scale(24960);     // this value is obtained by calibrating the scale with known weights; see the README for details
  scale.tare();               // reset the scale to 0

  lcd.init(); // initialize the LCD
  lcd.backlight(); // Включаем подсветку дисплея
   
  Serial.begin(9600);
  Serial.println("Start!");
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
  
}



