#include "LowPower.h" // pour la mise en veille
#include "HX711.h"
#include <RTClibExtended.h>
#include <SoftwareSerial.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SPI.h> // utilisation du SPI
#include <LoRa.h> // communication par module Lora fondé sur SX1278
#include <Wire.h> // utilisation du I2C
#include <EEPROM.h>
#define rxPin 5
#define txPin 4

const int mosfet = A0;
const int capteur1 = A1; //actuellement un capteur de temperature DS18B20
// const int capteur2 = A1;
// const int capteur3 = 8;
const int wake = 2;

int IDbase = 1; // sera inscrit dans EEPROM plus tard
int mode = 0; // mode de fonctionnement local : normal, apairage, recuperation capteurs annexes, detresse, etc...
float temperature;
int voltage;
int lumiere;
int count = 0;
int horloge[2];
int alarme1[2];
int alarme2[2];
int nalarmes;

float val1;
float val2;
float val3;
float val4;
float val5;
bool fin;
bool y;
bool o;


RTC_DS3231 RTC;      //RTC utilise

OneWire ds18x20[] = { 6, A1, 8 };
const int oneWireCount = sizeof(ds18x20)/sizeof(OneWire);
DallasTemperature sensor[oneWireCount];

SoftwareSerial Sigfox =  SoftwareSerial(rxPin, txPin); //configuration de la liaison avec le module sigfox
//Set to 0 if you don't need to see the messages in the console
#define DEBUG 1
//Message buffer
uint8_t msg[12];

HX711 scale(A3, A2); //initialisation du hx711

void setup() {
 // if(analogRead(A7) < 740) { // si tension de la batterie inférieur a 3.2 volts
  // envoi message de detresse batterie
  // code initial : on ne fait rien  
//}
//else{
  pinMode(mosfet, OUTPUT);
  pinMode(wake, INPUT);
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);

  Serial.begin(9600);
  Serial.println("lol");
  switsch_instruments(1); // allumage des instruments
  delay(100);

  scale.set_scale(2280.f); // valeur de calibration a définir
  scale.tare(); // tare de la balance
 
  DeviceAddress deviceAddress;
  for (int i = 0; i < oneWireCount; i++) {;
    sensor[i].setOneWire(&ds18x20[i]);
    sensor[i].begin();
    if (sensor[i].getAddress(deviceAddress, 0)) sensor[i].setResolution(deviceAddress, 12);
  }
  LoRa.setPins(10, 9, 3);
  
  Sigfox.begin(9600); // initialisation module sigfox et recuperation des informations utiles
  Serial.println("lol");
  delay(100);
  getID();
  delay(100);
  getPAC();

  Wire.begin();
  if (! RTC.begin()) {
    Serial.println("RTC introuvable");
    while (1);
  }
   if (RTC.lostPower()) {
    Serial.println("heure perdu, mise en place d une heure generique");
    // envoi message de demande heure
    RTC.adjust(DateTime(2000, 0, 0, 0, 0, 0));
    // heure generique 
  }
  
 // bienvenue(); // envoie du message de bienvenue
  switsch_instruments(1); // extinction des instruments
  //}
  RTC.armAlarm(1, false);
  RTC.clearAlarm(1);
  RTC.alarmInterrupt(1, false);
  RTC.armAlarm(2, false);
  RTC.clearAlarm(2);
  RTC.alarmInterrupt(2, false);
}

void loop() {
    //switsch_instruments(0); // extinction des instruments
    //mise en veille 
    // desarmement et effacement de toutes les alarmes
    RTC.setAlarm(ALM1_MATCH_SECONDS, 0, 0, 0, 0);   //creation alarme toutes les heures
    RTC.alarmInterrupt(1, true); // mise en interrupt
    RTC.armAlarm(1, true); // armement de l alarme
    RTC.writeSqwPinMode(DS3231_OFF); // mise off de la pin sqw pour les alarmes
    /*RTC.setAlarm(ALM2_MATCH_MINUTES, 0, 0, 0, 0);   //creation alarme toutes les heures
    RTC.alarmInterrupt(2, true); // mise en interrupt
    RTC.armAlarm(2, true); // armement de l alarme */
    Serial.print("veille");
    delay(10);
    attachInterrupt(0, reveil, LOW);
    switsch_instruments(0);
    delay(10); 
    LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);   //mise en veille profonde
    delay(10);
    switsch_instruments(1); // allumage des instruments
    RTC.armAlarm(1, false);
    RTC.clearAlarm(1);
    RTC.alarmInterrupt(1, false);
    RTC.armAlarm(2, false);
    RTC.clearAlarm(2);
    RTC.alarmInterrupt(2, false);
   // selection_mode();
    
    
   // Serial.print("fin veille");
    if(mode == 0){ // mode normal
      // recuperation des datas
      Serial.print("poids non calibre = ");
      delay(500);
      float poids = scale.get_value(10) / 50000;
      Serial.println(poids); 
      int poidsint = int(poids);
      int poidsdigit = abs(int(poids*100 - poidsint*100));
      poidsint = abs(poidsint);
      Serial.println(poidsint);
      Serial.println(poidsdigit);
      voltage = analogRead(A7); // une moyenne sera privilegiee plus tard
      Serial.print("tension de la batterie = ");
      Serial.println(voltage);
      lumiere = analogRead(A6); // une moyenne sera privilegiee plus tard
      int lumint = abs(int(lumiere / 10));
      int lumdigit = abs(int(lumiere - lumint*10));
      lumint = abs(lumint);
      Serial.print("lumiere = ");
      Serial.println(lumiere);

      float shttemp = 0.0;
      float shthum = 0.0;
      int shttempint = int(shttemp);
      int shttempdigit = abs(int(shttemp*100 - shttempint*100));
      shttempint = abs(shttempint);
      int shthumint = int(shthum);
      int shthumdigit = abs(int(shthum*100 - shthumint*100));
      shthumint = abs(shthumint);
      Serial.print("temperature sht30 = ");
      Serial.println(shttemp);
      Serial.print("humidite sht30= ");
      Serial.println(shthum);
      temperature = mesure(1); //temperature du capteur DS18B20
      int temperatureint = int(temperature);
      int temperaturedigit = abs(int(temperature*100 - temperatureint*100));
      temperatureint = abs(temperatureint);
      Serial.print("temperature = ");
      Serial.println(temperature);
      
      

      // creation du buffer 
      msg[0]=1; // type de message
      msg[1]=poidsint; // <255
      msg[2]=poidsdigit; // <255
      msg[3]=int(voltage / 4); // precision a ameliorer
      msg[4]=lumint;
      msg[5]=lumdigit;
      msg[6]=shttempint; // <255
      msg[7]=shttempdigit; // <255
      msg[8]=shthumint; // <255
      msg[9]=shthumdigit; // <255
      msg[10]=temperatureint; // <255
      msg[11]=temperaturedigit; // <255
      
      // envoie
      sendMessage(msg, 12);
      delay(7000); // il faut 6 secondes pour envoyer le message
    }
    else if(mode == 1){
      // mode apairage
      // code initial : on ne fait rien ici
      delay(500);
    }
    else if(mode == 2){
      Serial.print("go lora");
       if (!LoRa.begin(433E6)) {
        Serial.println("Starting LoRa failed!");
        while (1);
      }
      unsigned long t1 = millis();
      String paquet = "";
      while(millis() < t1 + 120000){
        int packetSize = LoRa.parsePacket();
        if (packetSize) {
          // received a packet
          Serial.print("Received packet '");
      
          // read packet
          while (LoRa.available()) {
            // Serial.print((char)LoRa.read());
            paquet = paquet + (char)LoRa.read();
          }
          Serial.print(paquet);
          // print RSSI of packet
          Serial.print("' with RSSI ");
          Serial.println(LoRa.packetRssi());

          float tsensor = paquet.toFloat();
          Serial.print(tsensor);
          int tsensorint = int(tsensor);
          int tsensordigit = abs(int(tsensor*100 - tsensorint*100));
          tsensorint = abs(tsensorint);

          msg[0]=3; // type de message
          msg[1]=1; // <255
          msg[2]=tsensorint; // <255
          msg[3]=tsensordigit; // precision a ameliorer
          msg[4]=0;
          msg[5]=0;
          msg[6]=0; // <255
          msg[7]=0; // <255
          msg[8]=0; // <255
          msg[9]=0; // <255
          msg[10]=0; // <255
          msg[11]=0; // <255
          
          sendMessage(msg, 12);
          delay(7000); // il faut 6 secondes pour envoyer le message
        }
      }
    }
    // d autres modes sont a prevoir plus tard

}

void switsch_instruments(int action){ // switsch les instruments de mesures et de communication pour allumage avec action = 1 ou extinction avec action = 0
  delay(10);
  if(action == 0){
    digitalWrite(mosfet, LOW); // extinction
    LoRa.end();
  }
  else{
    digitalWrite(mosfet, HIGH); // allumage
    LoRa.begin(433E6);
  }
  delay(500); // le temps d extinction des regimes transitoires 
}

float mesure(int k){ // renvoi la mesure en flottant issue du DS18B20
  delay(10);
  float temp = 0; // variable temperature
  int ok = 0;
  for(int i = 1; i <= 5; i++){ // on fait 5 mesures
    delay(10);
    sensor[k].requestTemperatures(); // Send the command to get temperatures
    delay(10);
    if(sensor[k].getTempCByIndex(0) != DEVICE_DISCONNECTED_C) 
     {
        temp = temp + sensor[k].getTempCByIndex(0);
        ok++;
     }
  }
  if(ok != 0)
  {
    return(temp / ok); // on fait la moyenne des 5 mesures
  }
  else
  {
    return(100);
  }
}

void reveil()        // commandes a executer en priorite apres le reveil
{
  delay(10);
  detachInterrupt(0);
  Serial.println("sortie");
  delay(250);
}

void selection_mode(){
  if(digitalRead(wake) == LOW){
    mode = 1;
    Serial.println("mode 1");
  }
  else{
    Serial.println("test");
    delay(500);
    DateTime now = RTC.now();
    if((now.minute(), DEC) == 0){
      mode = 0; // mode normal on fait les mesures classiques
      Serial.println("mode 0");
    }
    else{
      mode = 2;
      Serial.println("mode 2");
    }
  }
}

String getID(){
  String id = "";
  char output;

  Sigfox.print("AT$I=10\r");
  while (!Sigfox.available()){
     // on ne fait rien
  }

  while(Sigfox.available()){
    output = Sigfox.read();
    id += output;
    delay(10);
  }

  if(DEBUG){
    Serial.println("Sigfox Device ID: ");
    Serial.println(id);
  }

  return id;
  }


//Get PAC number
String getPAC(){
  String pac = "";
  char output;

  Sigfox.print("AT$I=11\r");
  while (!Sigfox.available()){
     // on ne fait rien
  }

  while(Sigfox.available()){
    output = Sigfox.read();
    pac += output;
    delay(10);
  }

  if(DEBUG){
    Serial.println("PAC number: ");
    Serial.println(pac);
  }

  return pac;
}


//Send Sigfox Message
void sendMessage(uint8_t msg[], int size){

  String status = "";
  char output;

  Sigfox.print("AT$SF=");
  for(int i= 0;i<size;i++){
    if(msg[i] < 16){
    Sigfox.print(String(0, HEX));  
    Sigfox.print(String(msg[i], HEX));
    if(DEBUG){
      Serial.print("Byte:");
      Serial.print(String(0, HEX)); 
      Serial.println(msg[i], HEX);
    }
    }
    else{
      Sigfox.print(String(msg[i], HEX));
    if(DEBUG){
      Serial.print("Byte:");
      Serial.println(msg[i], HEX);
    }
    }      
  }

  Sigfox.print("\r");

  while (!Sigfox.available()){
     // on ne fait rien
  }
  while(Sigfox.available()){
    output = (char)Sigfox.read();
    status += output;
    delay(10);
  }
  if(DEBUG){
    Serial.println();
    Serial.print("Status \t");
    Serial.println(status);
  }
}

void reception(unsigned long ms){
  Serial.print("reception");
  String h = ""; // heure horloge
  String m = ""; // minute horloge
  String s = ""; // seconde horloge
  String n = ""; // nombre d alarmes
  String j = ""; // heure alarme 1
  String k = ""; // minute alarme 1
  String l = ""; // seconde alarme 1
  String u = ""; // heure alarme 2
  String w = ""; // minute alarme 2
  String x = ""; // seconde alarme 2
  horloge[0] = 0;
  horloge[1] = 0;
  horloge[2] = 0;
  alarme1[0] = 0;
  alarme1[1] = 0;
  alarme1[2] = 0;
  alarme2[0] = 0;
  alarme2[1] = 0;
  alarme2[2] = 0;
  int variableWrite = 0;
  String IDS;
  String type;
  String vers;
  String valeur1;
  String valeur2;
  String valeur3;
  String valeur4;
  String valeur5;
  fin = false;
  y = false;
  o = false;
  unsigned long time1 = millis();
  while((time1 + ms) > millis() && fin == false){
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // received a packet
    Serial.print("Received packet '");
       while(LoRa.available()){ // tant qu il y a quelque chose dans le buffer
          char incomingByte = (char)LoRa.read();
          if(isAlpha(incomingByte)){
            switch(incomingByte){
              case 'i' : // debut de procedure d apairage
                variableWrite = 1; // correspond a ID
              break;
              case 'f' : // fin de communication
                fin = true;
              break;
              case 't' :
                variableWrite = 2; // correspond au type
              break;
              case 'v' : 
                variableWrite = 3; // corespond a la version
              break;
              case 'a' :
                variableWrite = 4;
              break;
              case 'b' : 
                variableWrite = 5;
              break;
              case 'c' : 
                variableWrite = 6;
              break;
              case 'd' : 
                variableWrite = 7;
              break;
              case 'e' : 
                variableWrite = 8;
              break;
              case 'h' : 
                variableWrite = 9;
              break;
              case 'm' : 
                variableWrite = 10;
              break;
              case 's' : 
                variableWrite = 11;
              break;
              case 'n' : 
                variableWrite = 12;
              break;
              case 'j' : 
                variableWrite = 13;
              break;
              case 'k' : 
                variableWrite = 14;
              break;
              case 'l' : 
                variableWrite = 15;
              break;
              case 'u' : 
                variableWrite = 16;
              break;
              case 'w' : 
                variableWrite = 17;
              break;
              case 'x' : 
                variableWrite = 18;
              break;
              case 'y' : 
                y = true;
              break;
            }
          }
          else{
            switch(variableWrite){
              case 0 :
                // le message est corrompu et a renvoyer 
                y = true;

              break;
              case 1 :
                IDS = IDS + incomingByte;
              break;
              case 2 :
                type = type + incomingByte;
              break;
              case 3 :
                vers = vers + incomingByte;
              break;
              case 4 :
                valeur1 = valeur1 + incomingByte;
              break;
              case 5 :
                valeur2 = valeur2 + incomingByte;
              break;
              case 6 :
                valeur3 = valeur3 + incomingByte;
              break;
              case 7 :
                valeur4 = valeur4 + incomingByte;
              break;
              case 8 :
                valeur5 = valeur5 + incomingByte;
              break;
              case 9 :
                h = h + incomingByte;
              break;
              case 10 :
                m = m + incomingByte;
              break;
              case 11 :
                s = s + incomingByte;
              break;
              case 12 :
                n = n + incomingByte;
              break;
              case 13 :
                j = j + incomingByte;
              break;
              case 14 :
                k = k + incomingByte;
              break;
              case 15 :
                l = l + incomingByte;
              break;
              case 16 :
                u = u + incomingByte;
              break;
              case 17 :
                w = w + incomingByte;
              break;
              case 18 :
                x = x + incomingByte;
              break;
              }
       }
       }
    int ID;
    EEPROM.get(4, ID);
    if(2 == IDS.toInt()){
    // enregistrement horloges et alarmes
    if(fin == true){ // si le message a une fin donc non corrompu
      if(o == true){ // si message de confirmation horloge ou apairage
          horloge[0] = h.toInt();
          horloge[1] = m.toInt();
          horloge[2] = s.toInt();
          
          nalarmes = n.toInt(); // nombre da alarmes
          if(nalarmes == 1){ // si 1 alarme
            alarme1[0] = j.toInt();
            alarme1[1] = k.toInt();
            alarme1[2] = l.toInt();
          }
          else if(nalarmes == 2){ // si 2 alarme
            alarme2[0] = u.toInt();
            alarme2[1] = w.toInt();
            alarme2[2] = x.toInt();
          }

      }
    } 
    int val1 = valeur1.toFloat();
    Serial.print("ID ");
    Serial.println(IDS.toInt());
    Serial.print("type ");
    Serial.println(type.toInt());
    Serial.print("version ");
    Serial.println(vers.toInt());
    Serial.print("valeur1 ");
    Serial.println(val1);
    Serial.println("fin ");
    Serial.println(fin);
    // print RSSI of packet
    Serial.print("' with RSSI ");
    Serial.println(LoRa.packetRssi());
    }
   
    }
    }
}
