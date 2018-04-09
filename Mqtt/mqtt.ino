#include <SoftwareSerial.h>

#define AnalogRead 2000 //период чтения напряжения, ms
#define DrawTime 1000 //период обновления экрана, ms
#define KeyReadTime 20 //период проверки нажатия клавиатуры

#define APN "internet.mts.ru" //
#define USER "mts" //
#define sURL "m23.cloudmqtt.com" //url сервера
#define INTERVALSEND 60000

SoftwareSerial SIM800(7, 8); //подгтавливаем программный serial порт на пинах 7 и 8

volatile int tyme = 0; //переменная прерываний
bool dt = false; //флаг обновления экрана
bool rt = false; //флаг чтения температуры
bool kr = 0; //флаг периода проверки клавиатуры

char aux_str[150];
unsigned long millissend=0;
uint8_t answer=0;

char url[150];
String surl="http://narodmon.ru/post.php/?";

//---------------- ФУНКЦИИ ----------------

// отправка AT-команд
int8_t sendATcommand(char* ATcommand, char* expected_answer, unsigned int timeout) {
   uint8_t x = 0,
           answer = 0;
   char response[150];
   unsigned long previous;

   memset(response, '\0', 150);    // Initialize the string
   delay(100);
   
   while (SIM800.available() > 0) 
      SIM800.read();    // Clean the input buffer
   
   Serial.print("SIM800 Send: ");
   Serial.println(ATcommand);
   SIM800.println(ATcommand);    // Отправка AT команды 
   
   x = 0;
   previous = millis();
   // this loop waits for the answer
   do {
     if(SIM800.available() != 0) {    
       // if there are data in the UART input buffer, reads it and checks for the asnwer
       response[x] = SIM800.read();
       x++;
       // check if the desired answer  is in the response of the module
       if (strstr(response, expected_answer) != NULL) {
         answer = 1;
       }
     }
    } while ((answer == 0) && ((millis() - previous) < timeout)); // время ожидания ответа   

    Serial.print("SIM800 answer: ");
    Serial.print(response);
    Serial.print(", ");
    Serial.println(answer);
    return answer;
}

void GPRSDisconnect() {
    //закрыть все  TCP/IP соединения
    sendATcommand("AT+CIPSHUT", "OK", 2000);
    // Deactivate the bearer context
    sendATcommand("AT+CGATT=0", "OK", 2000);
}

void GPRSinit() {
  //закрыть все соединения
  sendATcommand("AT+CIPSHUT", "OK", 2000);
  delay(500);
  
  sendATcommand("AT+SAPBR=0,1", "OK", 2000); //включаем режим GPRS
  //включаем режим GPRS
  sendATcommand("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"", "OK", 2000); //включаем режим GPRS
  //настриваем точку доступа APN
  snprintf(aux_str, sizeof(aux_str), "AT+SAPBR=3,1,\"APN\",\"%s\"", APN);
  sendATcommand(aux_str, "OK", 2000);
  snprintf(aux_str, sizeof(aux_str), "AT+SAPBR=3,1,\"USER\",\"%s\"", USER);
  sendATcommand(aux_str, "OK", 2000);
  snprintf(aux_str, sizeof(aux_str), "AT+SAPBR=3,1,\"PWD\",\"%s\"", USER);
  sendATcommand(aux_str, "OK", 2000);

   //устанавка GPRS связи
  while (sendATcommand("AT+SAPBR=1,1", "OK", 2000) == 0) {
    delay(2000);
  }

  Serial.println("GPRS OK");
  
  
  
  //активируем одиночное TCP/IP соединение
  sendATcommand("AT+CIPMUX=1", "OK", 2000);
  delay(500);
  //Уровень сигнала
  sendATcommand("AT+CSQ", "OK", 2000);
  delay(500);
  //Регистрация?
  sendATcommand("AT+CREG?", "OK", 2000);
  delay(500);
  //Проверка статуса GPRS?
  sendATcommand("AT+CGATT?", "+CGATT:1", 2000);
  delay(500);
  //настриваем точку доступа APN
//  snprintf(aux_str, sizeof(aux_str), "AT+CSTT=\"%s\"", APN, ",", USER, ",", USER);
  //sendATcommand(aux_str, "OK", 2000);
  sendATcommand("AT+CSTT=\"internet.mts.ru\",\"mts\",\"mts\"", "OK", 2000);
  //устанавка GPRS соединения
  delay(500);
  sendATcommand("AT+CIICR", "OK", 80000);
  delay(500);
  //IP запрашиваем
   sendATcommand("AT+CIFCR", "OK", 2000);
  delay(500);
  sendATcommand("AT+CIFCR", "OK", 2000);
  delay(500);
  sendATcommand("AT+CIFCR", "OK", 2000);
  delay(500);

  //Установка TCP соединения
  sendATcommand("AT+CIPSTART=0,\"TCP\",\"m23.cloudmqtt.com\",\"12843\"", "OK", 2000);
  sendATcommand("AT+CIPSTATUS=0", "OK", 2000);
  delay(500);
  
  
}

//---------------- System ФУНКЦИИ ----------------

void DrawMenu () {
  
}

void KeyPad () {
  
}

void ReadAnalog () {
   
}

//---------------- setup / loop / system ----------------

void setup() {
  OCR0A = 0xAF; //прерывание (system)
  TIMSK0 |= _BV(OCIE0A); //прерывание (system)

  Serial.begin(9600);
  SIM800.begin(9600); //запускаем программный serial порт

  Serial.println("Start!");
  
  //Модуль то работает?
  sendATcommand("OK", "OK", 2000);
  
  //включаем режим GPRS
  GPRSinit();
  
  delay(1000);
}

//прерывание, формирует мини операционную систему (system)
SIGNAL(TIMER0_COMPA_vect) {  
  tyme = tyme + 1;
  if (tyme % AnalogRead == 0) { rt = 1; };
  if (tyme % DrawTime == 0) { dt = 1; };
  if (tyme % KeyReadTime == 0) { kr = 1; };
}

void loop() {
  if (rt == 1) { rt = 0; ReadAnalog(); };  
  if (dt == 1) { dt = 0; DrawMenu(); };
  if (kr == 1) { kr = 0; KeyPad(); };

}



