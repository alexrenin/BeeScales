#include <HX711.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

#define scaleRead 500 //период чтения  чтения данных с тензодатчиков, ms
#define DrawTime 200 //период обновления экрана, ms
#define del 10 //делитель настроечного коэффициента
#define KeyReadTime 20 //период проверки нажатия клавиатуры

HX711 scale;   
LiquidCrystal_I2C lcd(0x3F, 16, 2);

volatile int tyme = 0; //переменная прерываний
bool dt = false; //флаг обновления экрана
bool scR = false; //флаг чтения данных с тензодатчиков
bool kr = 0; //флаг периода проверки клавиатуры

String screenString1 = "Starting...";
float scaleValue = 0;
float sumScaleValue = 0;
byte cntScale = 0;

void DrawMenu () {
  lcd.setCursor(0, 0);
  lcd.print("                ");
  lcd.setCursor(0, 0);
  lcd.print(screenString1);
  int z = 0;
  int result = 0;
  if (Serial.available() > 0) {
    while (Serial.available() > 0) {
      byte c1 = Serial.read(); 
      if ((c1<58)&&(c1>47)) {
        z = c1 - 48;
        result = result*10+z;
      } else {
        z = 0;
      }
    }
    scale.set_scale(result);
    Serial.println("scale.set_scale:");
    Serial.println(result);
  }
 
  
}

void KeyPad () {
 
}

void readScale () {
  scale.power_up();
  sumScaleValue += scale.get_units(10);
  scale.power_down();              // put the ADC in sleep mode
  
  cntScale++;
  
  if (cntScale>=2) {
    cntScale = 0;

    scaleValue = sumScaleValue/2;
    sumScaleValue = 0;
    
    screenString1 = String(scaleValue);
  
    Serial.print("\t| average:\t");
    Serial.println(scaleValue, 1);
  }
  
  
  
  
}

void setup() {
  //Serial.begin(9600);
  OCR0A = 0xAF; //прерывание
  TIMSK0 |= _BV(OCIE0A); //прерывание
  
  lcd.init(); // initialize the LCD
  lcd.backlight(); // Turn on the blacklight and print a message.
  lcd.print(screenString1);
  
  // HX711.DOUT  - pin #A1
  // HX711.PD_SCK - pin #A0
  scale.begin(A1, A0);
  scale.set_scale(24960);     // this value is obtained by calibrating the scale with known weights; see the README for details
  scale.tare();               // reset the scale to 0
  
  //Serial.println("Readings:");
}

SIGNAL(TIMER0_COMPA_vect) { //прерывание считывающие мощность
  tyme = tyme + 1;
  if (tyme % scaleRead == 0) { scR = 1; };
  if (tyme % DrawTime == 0) { dt = 1; };
  if (tyme % KeyReadTime == 0) { kr = 1; };
}

void loop()
{
  if ((scR == 1)) { scR = 0; readScale();}; 
  if (dt == 1) { dt = 0; DrawMenu(); };
  if (kr == 1) { kr = 0; KeyPad(); }; 
}



