/*********************************************************************************
Algoritimo de Controle para Alimentadores automáticos de Pets
Diego Telles 
27/05/2019
*********************************************************************************/

//incluindo bibliotecas
#define BLYNK_PRINT Serial
#include <ESP8266_Lib.h>                                                                  // placa wifi ESP-8266 vem com blynk
#include <BlynkSimpleShieldEsp8266.h>                                                     // blynk
#include "DHT.h"                                                                          // sensor de temperatura e umidade
#include <Wire.h>                                                                         // comunicação SCL SDA
#include <DS3231.h>                                                                       // RTC
#include <EEPROM.h>                                                                       //Armazenamento na memoria permanente do arduino (EEPROM)

//Informações de acesso a internet
char auth[] = "086b24b4350a42bb87d8b8f1c7a757c5";
char ssid[] = "VIVO-4788";
char pass[] = "C662344788";

// Hardware Serial no Mega
#define EspSerial Serial1

// Sua ESP8266 baud rate:
#define ESP8266_BAUD 115200
ESP8266 wifi(&EspSerial);

//Declarando constantes
#define DHTTYPE  DHT11                                                                    //Tipo do sensor DHT 11
const byte MotorPin = 00;                                                                 //portas digitais da saída para os reles 
const byte DHTPIN = 42;                                                                   //portas digitais sensor DHT - externo / ao lado do medidor de luz / colado na parede
const byte Trigger = 50, Echo = 48;                                                       //portas digitais sensor Ultrasonico para Nível
const byte buzzPin = 51;                                                                  //porta digital do buzz
const char daysOfTheWeek[][4] = {"Seg", "Ter", "Qua", "Qui", "Sex", "Sab", "Dom"};        //dias da semanda
const byte daysInMonth [] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};             //dias no mÊs
const float VelSomus = 0.000340;  

//Declarando variáveis
bool Modo = 0;                                                                            //Modo do program 0=Auto / 1=Manual
byte HoraComer = 0;                                                                       //Hora de ligar e desligar
byte ano = 0, mes = 0, data = 0, hora = 0, minu = 0, seg = 0, DoW = 0;                    //Leitura do horario e dia
byte mesUA = 0, diaUA = 0, horaUA = 0, minUA = 0, segUA = 0;                              //horario da ultima alimentação
byte statusMotor = 0;                     //Status de funcionamento 0=Off / 1=On
byte valAliBut = 0;  //Leitura acionamento dos botões
byte tmpDisp = 10;                                                                        //Tempo de disparo do som no sensor ultrasonico (us)
byte NivelMin = 19, NivelMax = 5;                                                         //valores de distância em m para nivel minimo e máximo do sensor
float nivel = 0;                                                                          //nivel de líquido em %
float temperatura = 0, umidade = 0;                                                       //Leitura Temperaturas DHT11
float TempoEcho = 0, dist = 0;                                                            //tempo de retorno do sinal sonoro e distância da superficie da água até o senso


/**************************   Criando Objetos   ***************************/

//Objeto blynk timer
BlynkTimer timer;

//objeto RTC
DS3231 RTC;

//Objetos dos sensores de T&U
DHT dht(DHTPIN, DHTTYPE);

//Attach virtual serial terminal to Virtual Pin V1
WidgetTerminal terminal(V4);

//Objeto DataTime PARA AJUSTE DE HORARIO: Ano, mes, data, hora, minuto, segundo e dia da semana (de 1 a 7)
DateTime dt(2019, 05, 19, 12, 24, 0, 7);



