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
