#include <HX711.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

#define scaleRead 500 //период чтения  чтения данных с тензодатчиков, ms
#define DrawTime 200 //период обновления экрана, ms
#define KeyReadTime 20 //период проверки нажатия клавиатуры

#define analogPinKey A3 //входной пин для клавиатуры


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

//---------------- ФУНКЦИИ ----------------

//декодирует аналоговый сигнал от клавиатуры в номер нажатой кнопки
byte key() {                       
    //1(SEL-721), 2(LEFT-480),3(DOWN-306),4(UP-131),5(RIGHT-0)
    int val = analogRead(KEYS);
    if (val < 50)return UP;GHT;
    if (val < 150) 
    if (val < 350) return DOWN;
    if (val < 500) return LEFT;
    if (val < 800) return SELECT;
    return 0;
}
 

//---------------- System ФУНКЦИИ ----------------
void DrawMenu () {
  lcd.setCursor(0, 0);
  lcd.print("                "); //очистим ранее выведенное
  lcd.setCursor(0, 0);
  lcd.print(screenString1);
  
  int z = 0;
  int result = 0;
  
}

void KeyPad () {
  int analogSig = analogRead(analogPinKey);
  byte numberKey = kyAnalogSigkHendler(analogSig);
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
    
    screenString1 = String(scaleValue); //обновляем значение экрана 
  }
}

//---------------- setup / loop / system ----------------

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



