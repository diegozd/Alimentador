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
const byte AliBut = 37;                                                                   //portas digitais dos botões
const byte MotorPin = 00;                                                                 //portas digitais da saída para os reles 
const byte DHTPIN = 42;                                                                   //portas digitais sensor DHT - externo / ao lado do medidor de luz / colado na parede
const byte Trigger = 50, Echo = 48;                                                       //portas digitais sensor Ultrasonico para Nível
const byte buzzPin = 51;                                                                  //porta digital do buzz
const char daysOfTheWeek[][4] = {"Seg", "Ter", "Qua", "Qui", "Sex", "Sab", "Dom"};        //dias da semanda
const byte daysInMonth [] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};             //dias no mÊs
const float VelSomus = 0.000340;                                                          //Velocidade da luz

//Declarando variáveis
bool Modo = 0;                                                                            //Modo do program 0=Auto / 1=Manual
bool AuxAli = 0, AuxAliBut = 0;                                                           //Auxiliar que impede a repetição de comandos
bool ComerAgora = 0;                                                                      //Auxiliar para definir se está na hora de comer
byte tmpDesc = 0;                                                                         //tempo a ser descontado no delay
byte duraAli = 10;                                                                        //Tempo de duração da alimentação em (s)
byte nHorarios = 3;                                                                       //numero de horarios no dia
byte HorariosComer [] = {6, 15, 22};                                                      //Hora de ligar e desligar
byte ano = 0, mes = 0, data = 0, hora = 0, minu = 0, seg = 0, DoW = 0;                    //Leitura do horario e dia
byte mesUA = 0, diaUA = 0, horaUA = 0, minUA = 0, segUA = 0;                              //horario da ultima alimentação
byte statusMotor = 0;                                                                     //Status de funcionamento 0=Off / 1=On
byte valAliBut = 0;                                                                       //Leitura acionamento dos botões
byte tmpDisp = 10;                                                                        //Tempo de disparo do som no sensor ultrasonico (us)
byte NivelMin = 19, NivelMax = 5;                                                         //valores de distância em m para nivel minimo e máximo do sensor
byte AuxNivel = 0;                                                                        //Auxiliar que impede a repetição de comandos
float nivel = 0, TempoEcho = 0, dist = 0;                                                 //nivel de líquido em %, tempo de retorno do sinal sonoro e distância da superficie da água até o senso
float temperatura = 0, umidade = 0;                                                       //Leitura Temperaturas DHT11
float Tcc = 0;                                                                            //Leitura Temperatura RTC



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



/************************************************************************/
/**************************      FUNÇÕES       **************************/
/************************************************************************/



/**************************   Funções Blynk   ***************************/




/**************************   Outras Funções   ***************************/

//função para emitir pulso sonoro para medição do nível
void DisparaPulsoUltrassonico()
{
  digitalWrite(Trigger, LOW);   //energiza sensor
  delayMicroseconds(tmpDisp);   //tempo de pulso
  digitalWrite(Trigger, HIGH);  //desenergiza sensor
}

//função do buzz
void buzzFunction(byte tmpBuzz)
{
  digitalWrite(buzzPin, HIGH);
  delay(tmpBuzz);
  digitalWrite(buzzPin, LOW);
}


/**********************************************************************/
/**************************      LOOP       ***************************/
/**********************************************************************/

void LoopReal()
{
  //zerando desconto no delay
  tmpDesc = 0;
  
  //Lendo tempo
  DateTime now = RTC.now();
  ano = now.year();
  mes = now.month();
  data = now.date();
  hora = now.hour();
  minu = now.minute();
  seg = now.second();
  DoW = now.dayOfWeek();
  
  
  /*************************************************/
  /*************      AUTOMAÇÕES       *************/
  /*************************************************/
 
  for (int i = 0; i < nHorarios ; i++)
  {
    if (hora == HorariosComer[i]) ComerAgora = 1;
    else ComerAgora = 0;
  }
  
  if ((ComerAgora == 1) && (minu == 0) && (AuxAli == 0))
  {//hora de alimentar as gatas
    digitalWrite(MotorPin, LOW);  //energizo Motor
    statusMotor = 1;
    AuxAli = 1;  
    
    mesUA = mes;
    diaUA = data;
    horaUA = hora;
    minUA = minu;
    segUA = seg;

    EEPROM.write(0, mesUA);
    EEPROM.write(1, diaUA);
    EEPROM.write(2, horaUA);
    EEPROM.write(3, minUA);
  }
  else if ((ComerAgora == 1) && (minu == 0) && (seg >= (segUA + duraAli)))
  {//passou o tempo desejado do funcionamento do motor
    digitalWrite(MotorPin, HIGH);  //desenergizo motor
    statusMotor = 0;
  }
  else if (ComerAgora == 0)
  {
    AuxAli = 0;  
  }
  
  
  
  /*************************************************/
  /*************    AÇÃO DOS BOTÕES    *************/
  /*************************************************/
  
  //botão da Alimentação
  valAliBut = digitalRead(AliBut);
  if (valAliBut == 1)
  {//botao foi acionado
    tmpDesc = 50;  
    buzzFunction(tmpDesc);    //buzz
    digitalWrite(MotorPin, LOW);  //energizo Motor
    statusMotor = 1;
    AuxAliBut = 1;  
    
    mesUA = mes;
    diaUA = data;
    horaUA = hora;
    minUA = minu;
    segUA = seg;

    EEPROM.write(0, mesUA);
    EEPROM.write(1, diaUA);
    EEPROM.write(2, horaUA);
    EEPROM.write(3, minUA);
  }
  else if ((AuxAliBut == 1) && (seg >= (segUA + duraAli)))
  {//passou o tempo desejado do funcionamento do motor
    digitalWrite(MotorPin, HIGH);  //desenergizo motor
    statusMotor = 0;
    AuxAliBut = 0;
  }
  
  
  
  /*************************************************/
  /************* LEITURA DOS SENSORES  *************/
  /*************************************************/
  
  //lendo Temperatura do RTC  
  RTC.convertTemperature();             //convert current temperature into registers 
  Tcc = RTC.getTemperature();           //read registers and display the temperature
  
  //Lendo Sensores de Temperatura e umidade
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  umidade = dht.readHumidity();
  
  // Read temperature as Celsius (the default)
  temperatura = dht.readTemperature();
  
  if ((minu == 1) && (AuxNivel < 5))
  {// sensor de nível só é lido no minuto 1 de cada hora 5 vezes seguidas
    DisparaPulsoUltrassonico();                                     //Envia pulso para o sensor
    TempoEcho = pulseIn(Echo,HIGH);                                 //Lê o tempo de retorno do sinal sonoro
    dist = (double) (VelSomus*TempoEcho)*100/2;                     //calcula distância do sensor para a lamina de água
    nivel = (double) 100*(dist - NivelMin)/(NivelMax - NivelMin);   //calcula nível de líquido
    tmpDesc = tmpDisp;
    AuxNivel++;
    
    //Passa infos ao Blynk
    Blynk.virtualWrite(V3, umidade);   
    Blynk.virtualWrite(V4, temperatura);   
    Blynk.virtualWrite(V5, nivel);   
  }
  else if (minu != 1)
  {
    AuxNivel = 0;
  }
  
  
  
  /*************************************************/
  /*************  MOSTRANDO VARIÁVEIS  *************/
  /*************************************************/
  
  //escrevendo msg no terminal serial
  Serial.print(data, DEC);
  Serial.print(F("/"));
  Serial.print(mes, DEC);
  Serial.print(F("/"));
  Serial.print(ano, DEC);
  Serial.print(F(" - "));
  Serial.print(daysOfTheWeek[DoW - 1]);
  Serial.print(F(" - "));
  Serial.print(hora, DEC);
  Serial.print(F(":"));
  Serial.print(minu, DEC);
  Serial.print(F(":"));
  Serial.println(seg, DEC);
  
  Serial.print(F("Dow = "));
  Serial.println(DoW);

  Serial.print(F("T (C) = "));
  Serial.println(temperatura);
  Serial.print(F("T_RTC (C) = "));
  Serial.println(Tcc);
  
  Serial.print(F("Uar (%) = "));
  Serial.println(umidade);
  
  Serial.print(F("Tempo Echo (us) = "));
  Serial.println(TempoEcho);
  Serial.print(F("Distancia sup (cm) = "));
  Serial.println(dist);
  Serial.print(F("Nivel Tq (%) = "));
  Serial.println(nivel);
  
  Serial.print(F("Status Motor = "));
  if(statusMotor == 0) Serial.println(F("Off"));
  else Serial.println(F("On"));

  Serial.print(F("valAliBut = "));
  Serial.println(valAliBut);

  Serial.println();
  
  //epera do loop
  delay(800 - tmpDesc);
  
}



/**********************************************************************/
/**************************      SETUP       **************************/
/**********************************************************************/

//algoritimo de inicialização
void setup() 
{
  //iniciando Serial
  Serial.begin(9600);
  
  //mensagens de Inicialização Serial
  Serial.println();
  Serial.println(F(" Controle"));
  Serial.println(F("de Cultura"));
  Serial.println(F("----------"));
  Serial.println();
  
  //Pinos de saida
  pinMode(buzzPin, OUTPUT);     //buzz
  pinMode(MotorPin, OUTPUT);    //rele
  pinMode(Trigger, OUTPUT);     //sensor de nível ultrasonico

  //Pinos entradas
  pinMode(AliBut, INPUT);      //botão
  pinMode(Echo, INPUT);        //sensor de nível ultrasonico

  //iniciando Motor
  digitalWrite(MotorPin, HIGH);  //desenergiza Motor
  
  //iniciando Trigger do sensor de nível ultrasonico
  digitalWrite(Trigger, HIGH);  //desenergiza sensor
  
  //Set ESP8266 baud rate
  EspSerial.begin(ESP8266_BAUD);
  delay(10);

  //Iniciando conexão Blynk
  Blynk.begin(auth, wifi, ssid, pass);
  
  //Som de Inicialização
  buzzFunction(100);
  
  //iniciando medidor de Temperatura e umidade
  dht.begin();
  
  //Inicializando cominição SLC SDA
  Wire.begin();
  
  //iniciando RTC
  RTC.begin();
  //RTC.adjust(dt); //Ajuste de data e horário como definido em 'dt'. Comentar esta linha se já estiver com o horário ajustado
  
  //iniciando informações de ultima Alimentação
  mesUA = EEPROM.read(0);
  diaUA = EEPROM.read(1);
  horaUA = EEPROM.read(2);
  minUA = EEPROM.read(3);
  
  //iniciando parametros de calibração
  NivelMin = EEPROM.read(4);
  NivelMax = EEPROM.read(5);
  tmpDisp = EEPROM.read(6);
  
  //lendo nivel
  DisparaPulsoUltrassonico();                                     //Envia pulso para o sensor
  TempoEcho = pulseIn(Echo,HIGH);                                 //Lê o tempo de retorno do sinal sonoro
  dist = (double) (VelSomus*TempoEcho)*100/2;                     //calcula distância do sensor para a lamina de água
  nivel = (double) 100*(dist - NivelMin)/(NivelMax - NivelMin);   //calcula nível de líquido
  Blynk.virtualWrite(V5, nivel);

  //lê temperatura
  temperatura = dht.readTemperature();
  Blynk.virtualWrite(V4, temperatura);
  
  // Setup a function to be called every second
  timer.setInterval(1000L, LoopReal);

  //Escreve no terminal do app a msg de inicialização
  terminal.clear();
  terminal.println(F("Blynk v" BLYNK_VERSION ": Dispositivo iniciado"));
  terminal.println(F("----------"));
  terminal.println(F("Alimentador Automático"));
  terminal.println(F("----------"));
  terminal.println();
  terminal.flush();
  
  //espera
  delay(2000);
}


//algoritimo de repetição
void loop() 
{
  Blynk.run();
  timer.run(); // Initiates BlynkTimer
}


