#include <DS3231.h>
#include <Wire.h>

#define AnalogRead 2000 //период чтения напряжения, ms
#define DrawTime 1000 //период обновления экрана, ms
#define KeyReadTime 20 //период проверки нажатия клавиатуры

#define WakePin 2

DS3231 Clock;

volatile int tyme = 0; //переменная прерываний
bool dt = false; //флаг обновления экрана
bool rt = false; //флаг чтения температуры
bool kr = 0; //флаг периода проверки клавиатуры

//clock variable
byte year, month, date, doW, hour, minute, second;
bool Century=false;
bool h12;
bool PM, g;


SIGNAL(TIMER0_COMPA_vect) { //прерывание считывающие мощность
  tyme = tyme + 1;
  if (tyme % AnalogRead == 0) { rt = 1; };
  if (tyme % DrawTime == 0) { dt = 1; };
  if (tyme % KeyReadTime == 0) { kr = 1; };
}

void setTime(byte sYear, byte sMonth, byte sDate,
            byte sDoW, byte sHour, byte sMinute,
            byte sSecond) {
  Serial.print("Set time: "); 
  Serial.print("20");
  Serial.print(sYear);
  Serial.print(".");
  Serial.print(sMonth);
  Serial.print(".");
  Serial.print(sDate);
  Serial.print(" ");
  Serial.print(sHour);
  Serial.print(":");
  Serial.println(sMinute);
  
//  Clock.setClockMode(h12);  // set to 24h
 
  Clock.setYear(sYear);
  Clock.setMonth(sMonth);
  Clock.setDate(sDate);
  Clock.setDoW(sDoW);
  Clock.setHour(sHour);
  Clock.setMinute(sMinute);
  Clock.setSecond(sSecond);  
}

void getTime() {
  year=Clock.getYear();
  month=Clock.getMonth(Century);
  date=Clock.getDate();
  hour=Clock.getHour(h12,PM); 
  minute=Clock.getMinute();  
  second=Clock.getSecond();
}

void DrawMenu () {
  getTime();
  Serial.print("20");
  Serial.print(year);
  Serial.print(".");
  Serial.print(month);
  Serial.print(".");
  Serial.print(date);
  Serial.print(" ");
  Serial.print(hour);
  Serial.print(":");
  Serial.print(minute);
  Serial.print(":");
  Serial.println(second);

  // Display Alarm 1 information
  byte ADay, AHour, AMinute, ASecond, ABits;
  bool ADy, A12h, Apm;
  
  Serial.print("Alarm 1: ");
  Clock.getA1Time(ADay, AHour, AMinute, ASecond, ABits, ADy, A12h, Apm);
  Serial.print(ADay, DEC);
  if (ADy) {
    Serial.print(" DoW");
  } else {
    Serial.print(" Date");
  }
  Serial.print(' ');
  Serial.print(AHour, DEC);
  Serial.print(' ');
  Serial.print(AMinute, DEC);
  Serial.print(' ');
  Serial.print(ASecond, DEC);
  Serial.print(' ');
  if (A12h) {
    if (Apm) {
      Serial.print('pm ');
    } else {
      Serial.print('am ');
    }
  }
  if (Clock.checkAlarmEnabled(1)) {
    Serial.print("enabled");
  }
  Serial.print('\n');
    
}

void setSleepTimer(int mins){
//  uint8_t hours = mins / 60;
//  uint8_t mins_left = mins - hours * 60;
//  dt = Clock.getDateTime();
//  uint8_t hnow = dt.hour;
//  uint8_t mnow = dt.minute;
//  uint8_t sec = dt.second;
//  mnow += mins_left;
//  if (mnow >= 60) {
//    mnow -= 60;
//    hnow += 1;
//  }
//  hnow += hours;
//  if (hnow > 23) {
//    hnow -= 24;
//  }
//  clock.setAlarm1(0, hnow, mnow, sec, DS3231_MATCH_H_M_S);
}

void KeyPad () {
  bool g2 = digitalRead(WakePin);
  if (g != g2) {
    g = g2;
    Serial.print("Pin2: ");
    Serial.println(g2); 
  }
}

void ReadAnalog () {
   
}

void setup() {
  OCR0A = 0xAF; //прерывание
  TIMSK0 |= _BV(OCIE0A); //прерывание

  Serial.begin(9600);
  Wire.begin();

  pinMode(WakePin, INPUT);
  Clock.turnOnAlarm(1);

  byte A1Day = 4;
  byte A1Hour = 20;
  byte A1Minute = 30;
  byte A1Second = 0; 
  byte AlarmBits = 0;
  bool A1Dy = false;
  
  Clock.setA1Time(A1Day, A1Hour, A1Minute, A1Second, AlarmBits, A1Dy, h12, PM);
}

void loop() {
  if (rt == 1) { rt = 0; ReadAnalog(); };  
  if (dt == 1) { dt = 0; DrawMenu(); };
  if (kr == 1) { kr = 0; KeyPad(); };
}




