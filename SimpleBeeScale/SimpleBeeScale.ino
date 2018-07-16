#include <HX711.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>

#define ScaleRead 150 //период чтения  чтения данных с тензодатчиков, ms
#define DrawTime 1000 //период обновления экрана, ms
#define KeyReadTime 20 //период проверки нажатия клавиатуры, ms

#define KeypadPin A2 //За сколько измерений усреднять значения веса

#define CntAvScaleRead 5 //За сколько измерений усреднять значения веса
#define KeypadMaratory 300 //время маротория на считывание значений клавиатуры для исключения 
                          //повторного нажатия, ms

#define SELECT 1 //коды клавиш
#define UP 2 //коды клавиш
#define DOWN 3 //коды клавиш
#define LEFT 4 //коды клавиш
#define RIGHT 5 //коды клавиш

//19 ячейка EEPROM - хранит порядковый номер записанного веса

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
float scaleValueRTU = 0;

//keypad variable
byte curPresKey = 0; //код текущей нажатой кнопки
bool keyPressed = 0; //Флан мартория обработки нажатия для исключения повторного нажатия
int lastPressedTime = 0; //время последнего нажатия


//menu / archive variable
bool flagArchive = 0; //флаг нахождения в режиме работы с архивом
byte adress = 0; //адресс текущей ячейки EEPROM для записи
uint8_t charge0[8] =  //символ заряда батареи
{
  B11111,
  B10000,
  B10111,
  B10111,
  B10111,
  B10111,
  B10000,
  B11111,
};
uint8_t discharge0[8] =  //символ заряда батареи
{
  B11111,
  B10000,
  B10000,
  B10000,
  B10000,
  B10000,
  B10000,
  B11111,
};
uint8_t charge1[8] =  //символ заряда батареи
{
  B11111,
  B00000,
  B11111,
  B11111,
  B11111,
  B11111,
  B00000,
  B11111,
};
uint8_t discharge1[8] =  //символ заряда батареи
{
  B11111,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B11111,
};
uint8_t charge2[8] =  //символ заряда батареи
{
  B11111,
  B00001,
  B11101,
  B11101,
  B11101,
  B11101,
  B00001,
  B11111,
};
uint8_t discharge2[8] =  //символ заряда батареи
{
  B11111,
  B00001,
  B00001,
  B00001,
  B00001,
  B00001,
  B00001,
  B11111,
};
//---------------- ФУНКЦИИ ----------------
void writeToArchive (byte adress1, float value) {
  if (adress > 166) return;
  
  int adr2 = 20+adress1*6;
  int intValue = round(value*100);
  
  EEPROM.write(adr2, intValue);
  EEPROM.write(adr2+1, intValue>>8); 
//  EEPROM.write(adr2+2, (year<<2) | (month>>2));
//  EEPROM.write(adr2+3, ((month<<6) | date));  
//  EEPROM.write(adr2+4, hour);
//  EEPROM.write(adr2+5, minute); 
  EEPROM.write(19, adress1);
}
void readFromArchive(byte adress1) {
  int adr2 = 20+adress1*6;
  
  scaleValue = EEPROM.read(adr2) | (EEPROM.read(adr2+1)<<8);
//  year=EEPROM.read(adr2+2)>>2;
//  month=(EEPROM.read(adr2+3)>>6) | ((EEPROM.read(adr2+2)&0x3)<<2);
//  date=EEPROM.read(adr2+3) & 0x3F;
//  hour=EEPROM.read(adr2+4);
//  minute=EEPROM.read(adr2+5);
}


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
void drawLevelCharge (byte level) {
  if (level > 4) return;

  if (level > 0) { lcd.write(0); } else { lcd.write(1); }
  if (level > 1) { lcd.write(2); } else { lcd.write(3); }
  if (level > 2) { lcd.write(2); } else { lcd.write(3); }
  if (level > 3) { lcd.write(4); } else { lcd.write(5); }
  
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
void drawWeight() {
  lcd.setCursor(0, 1);
  lcd.print("        "); //очистим ранее выведенное
  lcd.setCursor(0, 1);
  
  drawNumber(scaleValue, 2);
  lcd.print(" kg");  
}

void DrawMenu () {
  lcd.setCursor(13, 0);
  lcd.print("   "); //очистим ранее выведенное
  lcd.setCursor(12, 1);
  lcd.print("    "); //очистим ранее выведенное
  if (flagArchive) {
    lcd.setCursor(13, 0);
    if (adress<10)  lcd.print(0);
    if (adress<100) lcd.print(0);
    lcd.print(adress);
    
  } else {
    lcd.setCursor(12, 1);
    drawLevelCharge(0);
  }

  drawWeight();
}

void KeyPad () {
 if (!keyPressed){ //мороторий на нажатие?
    //ветка "нет"
   byte keyCode = key(); 

   if (keyCode != 0) {
    keyPressed = 1;
    lastPressedTime = millis(); 
   }
   
   switch (keyCode) {
    case SELECT:
      //запись в архив
      Serial.println("Select");
      if (!flagArchive) {
        adress = EEPROM.read(19); //последняя записанная ячейка
        adress++;
        writeToArchive(adress, scaleValue);
        flagArchive=!flagArchive;
        Serial.println("Write");
      }
      
      break;
    case UP:
      Serial.println("Up");
      if (flagArchive) {
        adress++;
        if (adress == 167) {adress = 1;}
        readFromArchive(adress);
      }
      break;
    case DOWN:
      Serial.println("Down");
      if (flagArchive) {
        adress--; 
        if (adress == 0) {adress = 166;}
        readFromArchive(adress);
      }
      break;
    case LEFT:
      break;
    case RIGHT:
      //вход в архив
      flagArchive=!flagArchive;
      adress = EEPROM.read(19); //последняя записанная ячейка
      
      if (!flagArchive) {
        Serial.println("Archiv OFF");
      } else {
        readFromArchive(adress);
        Serial.println("Archiv IN");
      }
      
      break;
    default:
      keyPressed = 0;
      break;
   }
 } else {
  int currentTime = millis();
  if ((currentTime-lastPressedTime)>KeypadMaratory) {
    keyPressed = 0;
  }   
 }
}

void ReadScale () {
  readDigits[cntReadDigits] = scale.get_units();
  cntReadDigits++;
  
  if (cntReadDigits > 4) {
    sumScaleValue += GetMedian (readDigits);
    cntSumScale++;
    cntReadDigits = 0;

    if (cntSumScale >= CntAvScaleRead) {
      scaleValueRTU = sumScaleValue/cntSumScale;
      //  Serial.println(scaleValue);
      cntSumScale = 0;
      sumScaleValue = 0;
    }
  }

  if (!flagArchive) {
    scaleValue = scaleValueRTU;
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
//  lcd.createChar(0, charge0);
//  lcd.createChar(1, discharge0);
//  lcd.createChar(2, charge1);
//  lcd.createChar(3, discharge1);
//  lcd.createChar(4, charge2);
//  lcd.createChar(5, discharge2);
  lcd.backlight(); // Включаем подсветку дисплея
  lcd.clear();
  
  pinMode(KeypadPin, INPUT);
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



