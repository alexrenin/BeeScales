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
 
  Clock.setClockMode(h12);  // set to 24h
 
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
//  getTime();
//  Serial.print("20");
//  Serial.print(year);
//  Serial.print(".");
//  Serial.print(month);
//  Serial.print(".");
//  Serial.print(date);
//  Serial.print(" ");
//  Serial.print(hour);
//  Serial.print(":");
//  Serial.print(minute);
//  Serial.print(":");
//  Serial.println(second);
//
//  // Display Alarm 1 information
//  byte ADay, AHour, AMinute, ASecond, ABits;
//  bool ADy, A12h, Apm;
//  
//  Serial.print("Alarm 1: ");
//  Clock.getA1Time(ADay, AHour, AMinute, ASecond, ABits, ADy, A12h, Apm);
//  Serial.print(ADay, DEC);
//  if (ADy) {
//    Serial.print(" DoW");
//  } else {
//    Serial.print(" Date");
//  }
//  Serial.print(' ');
//  Serial.print(AHour, DEC);
//  Serial.print(' ');
//  Serial.print(AMinute, DEC);
//  Serial.print(' ');
//  Serial.print(ASecond, DEC);
//  Serial.print(' ');
//  if (A12h) {
//    if (Apm) {
//      Serial.print('pm ');
//    } else {
//      Serial.print('am ');
//    }
//  }
//  if (Clock.checkAlarmEnabled(1)) {
//    Serial.print("enabled");
//  }
//  Serial.print('\n');
//  if (Clock.checkIfAlarm(1) ) 
//  {
//    Serial.println("Alarm 1 Triggered");
//  }
}

void setSleepTimer(int mins){
  getTime();
  byte A1Day = date;
  byte A1Hour = hour;
  byte A1Minute = minute;
  byte A1Second = second; 
  byte AlarmBits = 0x0; //Match Type Values. 0=Day/Date, 8=Hrs/Min/Sec, 12=Min/Sec, 14=Sec, 15=Every Sec
  bool A1Dy = false; //date or day of week

  A1Minute = A1Minute + mins;

  if (A1Minute >= 60) {
    A1Minute = A1Minute - 60;
    A1Hour = A1Hour + 1;
  }

  Clock.setA1Time(A1Day, A1Hour, A1Minute, A1Second, AlarmBits, A1Dy, h12, PM);
  Clock.turnOnAlarm(1);
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

void weakUP() {
    bool g2 = digitalRead(WakePin);
    Serial.print("");
    Serial.print("Pin2: ");
    Serial.println(g2); 
    Serial.print(" ");
}

void setup() {
  attachInterrupt(0, weakUP, CHANGE); //внешнее прерывание от RTC
  
  OCR0A = 0xAF; //прерывание
  TIMSK0 |= _BV(OCIE0A); //прерывание

  Serial.begin(9600);
  Wire.begin();

  pinMode(WakePin, INPUT);
  digitalWrite(WakePin, HIGH);
//  Clock.turnOnAlarm(1);

  setSleepTimer(1);
}

void loop() {
  if (rt == 1) { rt = 0; ReadAnalog(); };  
  if (dt == 1) { dt = 0; DrawMenu(); };
  if (kr == 1) { kr = 0; KeyPad(); };
}




