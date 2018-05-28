#include <HX711.h>
#include "LedControl.h" 

#define scaleRead 500 //период чтения  чтения данных с тензодатчиков, ms
#define DrawTime 200 //период обновления экрана, ms
#define KeyReadTime 20 //период проверки нажатия клавиатуры

HX711 scale;  
//Pin 7 to DIN, 6 to Clk, 5 to LOAD, no.of devices is 1  
LedControl screen = LedControl(7,6,5,1); 

volatile int tyme = 0; //переменная прерываний
bool dt = false; //флаг обновления экрана
bool scR = false; //флаг чтения данных с тензодатчиков
bool kr = 0; //флаг периода проверки клавиатуры

float scaleValue = 0;
float sumScaleValue = 0;
byte cntScale = 0;

//---------------- ФУНКЦИИ ----------------
void drawArray (byte digits[5], byte dotNumb, bool mines) {
  byte digitNumb = 7;
  
  if (mines) {
    screen.setRow(0,digitNumb,0b00000001);
    digitNumb--;
  }

  for (byte i = 0; i < 5; i++) {
    byte digit = digits[i];
    bool flag = false;
    
    if (i == dotNumb) {flag = true; }
    
    if (!(digit == 0 && i < dotNumb)) {
      screen.setDigit(0,digitNumb,digit,flag); 
      digitNumb--; 
    }
    
    
  }
}

void drawNumber (float number) {
  int intNumber = abs((int) 100*number);
  byte digitCnt = 4;
  
  byte digits[5] = {0, 0, 0, 0, 0};
  byte dotNumb = 2;
  bool mines = false;
  
  if (number < 0) {
    mines = true;
  }
    
  while (intNumber > 0) {
   digits[digitCnt] = intNumber % 10;
   intNumber = intNumber / 10;
   digitCnt--;          
  } 

  drawArray (digits, dotNumb, mines);

}

//---------------- System ФУНКЦИИ ----------------
void DrawMenu () {
  Serial.println(scaleValue);
  
  screen.clearDisplay(0); // Очистить дисплей
  drawNumber(scaleValue); 
}

void KeyPad () {
  
}

void ReadScale () {
  scale.power_up();
  sumScaleValue += scale.get_units(10);
  scale.power_down();              // put the ADC in sleep mode
  
  cntScale++;
  
  if (cntScale>=2) {
    cntScale = 0;

    scaleValue = sumScaleValue/2;
    sumScaleValue = 0;
    
    
  }
}

//---------------- setup / loop / system ----------------

void setup() {
  Serial.begin(9600);
  OCR0A = 0xAF; //прерывание
  TIMSK0 |= _BV(OCIE0A); //прерывание
  
  screen.shutdown(0,false); // Включить дисплей
  screen.setIntensity(0,5);// Установить уровень яркости (0 is min, 15 is max) 
  screen.clearDisplay(0); // Очистить дисплей
  
  // HX711.DOUT  - pin #A1
  // HX711.PD_SCK - pin #A0
  scale.begin(A1, A0);
  scale.set_scale(24960);     // this value is obtained by calibrating the scale with known weights; see the README for details
  scale.tare();               // reset the scale to 0
  
}

//прерывание, формирует мини операционную систему (system)
SIGNAL(TIMER0_COMPA_vect) { 
  tyme = tyme + 1;
  if (tyme % scaleRead == 0) { scR = 1; };
  if (tyme % DrawTime == 0) { dt = 1; };
  if (tyme % KeyReadTime == 0) { kr = 1; };
}

void loop()
{
  if ((scR == 1)) { scR = 0; ReadScale();}; 
  if (dt == 1) { dt = 0; DrawMenu(); };
  if (kr == 1) { kr = 0; KeyPad(); }; 
}



