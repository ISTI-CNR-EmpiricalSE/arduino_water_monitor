#include <ArduinoMqttClient.h>
#include <WiFiClientSecure.h>
#include <FlowMeter.h>
#include <LiquidCrystal_I2C.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <ArduinoJson.h>
#include "time.h"
#include "sntp.h"
#include "WiFi.h"

/*
 * Questo programma caricato nell'ESP32 deve occuparsi di fare da publisher
 * nel broker MQTT per rendere disponibile l'informazione all'interno del topic
 * 
 * author: @rosario_parisi
 * 10/05/22
 */

 /* ---------------------------DEVICE STATUS---------------------------------------------------*/
const int CONNECTION_LOST = 1;
const int GENERAL_PROBLEM = 2;
int status_dispositivo = 0;

/* ---------------------------Scheduler Info---------------------------------------------------*/

const unsigned long eventIntervalSensorFlow = 1000;   // event interval to handle sensor reading
const unsigned long eventIntervalSendValues = 10000;  // event interval to handle values sending
const unsigned long eventIntervalTempSensor = 5000;   // event interval to handle values sending

unsigned long nowMillis;
unsigned long prevMillisIntervalSensorFlow;
unsigned long prevMillisIntervalSendValues;
unsigned long prevMillisIntervalTempSensor;

/*----------------------------WiFi info--------------------------------------------------------*/
const char* ssid = "your_ssid_name";
const char* password = "your_pwd";
WiFiClientSecure wifiClient; 
MqttClient mqttClient(wifiClient);

int flag_trovato = 0; // indica se la rete wifi è stata trovata nella scansione

 /*---------------------------Time Info-------------------------------------------------------*/
const char* ntpServer1 = "pool.ntp.org";
const char* ntpServer2 = "time.nist.gov";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;

const char* time_zone = "CET-1CEST,M3.5.0,M10.5.0/3";  // TimeZone rule for Europe/Rome

/*--------------------------Broker info-------------------------------------------------------*/
const char* broker = "your_broker_address";
int port = 8883;
const char* username_broker = "your_username_broker";
const char* pwd_broker = "your_password_broker";
const char* topic = "ArduinoWaterSensors/Sensori/1";
const char* ca_cert = \
"-----BEGIN CERTIFICATE-----\n" \
" your certificate 
  to publish to the topic
" \
"-----END CERTIFICATE-----\n";

/* ---------------------------Water Flow Sensor Info-------------------------------------------*/

FlowMeter *Meter;
FlowSensorProperties YFS201 = {30.0f, 2.25f, {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}};
const unsigned long period = 1000;

/* ---------------------------Water Level Sensor Info------------------------------------------*/

const int altezza_1 = 3;  // 3 cm
const int altezza_2 = 5;  // 5 cm
const int altezza_3 = 10; // 10 cm

/* Digital pin to receive signal from water sensor */
const int pin_S1 = 27;
const int pin_S2= 26;
const int pin_S3 = 25;

/* Variable to capture water level from analog_read */
int livello_raggiunto;
int livello_raggiunto_prec = 0;

/* Setup display1 e display2 (address, num char, numr rows) */
LiquidCrystal_I2C lcd_1(0x27,16,2);
LiquidCrystal_I2C lcd_2(0x25,16,2);

/*-----------------------------Ultrasonic Sensor Reading---------------------------------------*/
int distance_cm_captured;
const int distance_cm_from_sensor = 30 ; // cm
//const int distance_cm_from_sensor = 34 ; // cm
int value_profondita = 0;
const int triggerPin = 14;
const int echoPin = 12;

/* ---------------------------Temperature & Photoresistor Reading------------------------------*/
/* env. temp */
const int pinTemperatura = 35;
float tempC; // temperatura in Celsius
int tempVal;

/* env. brightness */
int fotoresistorValue = 0;

/* water temp */
const int pinSensoreTempAcqua = 5;
float tempCWater;
OneWire oneWire(pinSensoreTempAcqua);
DallasTemperature sensorTmpAcqua(&oneWire);

const int pinLedWifiStatus = 18;

/* -------------------------- Prototypes -------------------------------------------------------*/

String printLocalTime();                                              // returns a string represent today date
float convertiValoreSensore(int value);                               // converte in °C il valore del sensore di temperatura
void MeterISR();                                                      // interrupt routine function
void scanWifi();                                                      // scansione delle reti WIFI
int setupWifi();                                                      // connessione alla rete WIFI
void setupBrokerConnection();                                         // connessione al broker MQTT
void jsonMessageToSend();                                             // invio messaggio json con dati al topic MQTT
int letturaSogliaRaggiunta();                                         // lettura livello acqua raggiunto (1,2,3)
void updateLcdDisplay(int livello_raggiunto, LiquidCrystal_I2C lcd);  // aggiorno informazioni display LCD
void setDisplayInfo(LiquidCrystal_I2C lcd);                           // settaggio info display
void setDisplayLogo(LiquidCrystal_I2C lcd);                           // settaggio logo prodotto display LCD
long readUltrasonicDistance(int triggerPin, int echoPin);             // lettura profondità acqua sensore ultrasuoni 

void setup() {

  Serial.begin(115200);
  Serial.print("Setup");

  // setup led wifi
  pinMode(pinLedWifiStatus, OUTPUT);

  // config time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2);

  // Sensor1 YF-S201
  Meter = new FlowMeter(digitalPinToInterrupt(33), YFS201, MeterISR, RISING);

  // Setup pin livelli profondità
  gpio_reset_pin((gpio_num_t) pin_S1);
  gpio_reset_pin((gpio_num_t) pin_S2);
  gpio_reset_pin((gpio_num_t) pin_S3);
  pinMode(pin_S1, INPUT_PULLUP);
  pinMode(pin_S2, INPUT_PULLUP);
  pinMode(pin_S3, INPUT_PULLUP);

  /* Setting Display 1 */
  setDisplayInfo(lcd_1);
 
  /* Setting Display2 */
  setDisplayLogo(lcd_2);

  // ultrasonic sensor
  pinMode(triggerPin, OUTPUT); // Sets the trigPin as an OUTPUT
  pinMode(echoPin, INPUT);

  // water temp C
  sensorTmpAcqua.begin();
  
}

void loop() {

   // riconnessione automatica in caso di perdita della connessione
   if(flag_trovato==0 || WiFi.status()==WL_CONNECTION_LOST || WiFi.status()==WL_DISCONNECTED){
     status_dispositivo = CONNECTION_LOST;
     digitalWrite(pinLedWifiStatus, LOW);
     scanWifi();
   }

   // tiene attiva la connessione col broker 
   mqttClient.poll(); 

   // millisecondi dall'inizio dell'esecuzione
   nowMillis = millis();

   //-----------------------------------------------------|| letture del sensore YSF201 ogni 1000ms
   if( nowMillis - prevMillisIntervalSensorFlow > eventIntervalSensorFlow){
     Meter->tick(period);
     prevMillisIntervalSensorFlow = nowMillis;
   }

   // invio dei dati al topic ogni 10000ms
   if( nowMillis - prevMillisIntervalSendValues > eventIntervalSendValues){
     jsonMessageToSend();
     prevMillisIntervalSendValues = nowMillis;
    
   }

   //-----------------------------------------------------|| letture sensore livello acqua 
   livello_raggiunto = letturaSogliaRaggiunta();

   // update LCD display
   if(livello_raggiunto_prec!=livello_raggiunto){
      updateLcdDisplay(livello_raggiunto, lcd_1);
     
   }
   
   livello_raggiunto_prec = livello_raggiunto;
   

   //-----------------------------------------------------|| lettura temperatura, luminosità, profondità acqua e temp.acqua (5000ms)
   if( nowMillis - prevMillisIntervalTempSensor > eventIntervalTempSensor){

      tempVal = analogRead(pinTemperatura);
      tempC = convertiValoreSensore(tempVal);
      prevMillisIntervalTempSensor = nowMillis;
      Serial.print("Temp terra");
      Serial.println(tempC); 

      // fotoresistor reading
      fotoresistorValue = analogRead(32);

      // ultrasonic sensor reading
      distance_cm_captured = 0.017155*readUltrasonicDistance(triggerPin, echoPin);
      value_profondita = distance_cm_from_sensor-distance_cm_captured;
      Serial.print("Distance");
      Serial.println(value_profondita);

      // water temp reading
      sensorTmpAcqua.requestTemperatures();
      tempCWater = sensorTmpAcqua.getTempCByIndex(0);
      Serial.print("Temp acqua");
      Serial.println(tempCWater);
   }

}

long readUltrasonicDistance(int triggerPin, int echoPin){

  //pinMode(triggerPin, OUTPUT);
  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);

  // sets trigger pin to HIGH state for 10 microsec
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerPin, LOW);
  //pinMode(echoPin, INPUT);

  // conta il tempo che impiega l'impulso per andare e ritornare
  return pulseIn(echoPin, HIGH); 
   
}

void updateLcdDisplay(int livello_raggiunto, LiquidCrystal_I2C lcd){

  setDisplayInfo(lcd);
  lcd.setCursor(14,0);
  lcd.print(livello_raggiunto);
  lcd.setCursor(11,1);
  
  if(livello_raggiunto==1){
    lcd.print(altezza_1);
  }else if(livello_raggiunto==2){
    lcd.print(altezza_2);
  }else if(livello_raggiunto==3){
    lcd.print(altezza_3);
  }else if(livello_raggiunto==0){
    lcd.print(0);
  }
  
}

void setDisplayInfo(LiquidCrystal_I2C lcd){

  lcd.begin();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Liv.Raggiunto:0");
  lcd.setCursor(0,1);
  lcd.print("Altezza cm:0");
  
}

void setDisplayLogo(LiquidCrystal_I2C lcd){

  lcd.begin();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("**Lev.Measure***");
  lcd.setCursor(0,1);
  lcd.print("** SYS 2022  ***");
  
}

void setupBrokerConnection(){

  mqttClient.setId("ClientArduinoESP32");
  mqttClient.setUsernamePassword(username_broker, pwd_broker);

  if(!mqttClient.connect(broker, port)){
    Serial.print("Connessione al Broker MQTT fallita! Errore: ");
    Serial.println(mqttClient.connectError());
    scanWifi();
    while(1);
  }else{
    Serial.println("Connessione al Broker MQTT riuscita :)");
    status_dispositivo = 0;
  }
  
}

void jsonMessageToSend(){

  StaticJsonDocument<200> doc; // alloca nello stack 200bytes di memoria
  JsonObject object_message = doc.to<JsonObject>();

  object_message["Date"] = printLocalTime();      // mettere timestamp
  object_message["Long"] = 43.71;
  object_message["Lat"] = 10.42;

  if(status_dispositivo!=0){

    object_message["Status"] = status_dispositivo;  // 0 normal, 1 error
    
  }else{

    object_message["CurFlowRate"] = float(Meter->getCurrentFlowrate());
    object_message["TotVolumeRate"] = float(Meter->getTotalVolume());
    object_message["Temp"] = tempC;
    object_message["TempWater"] = tempCWater;
    object_message["LivAcqua"] = livello_raggiunto;
    object_message["ProfAcqua"] = value_profondita;
    object_message["LivLuce"] = fotoresistorValue;  // valore del fotoresistore

  }
  
  char buffer[256];
  serializeJson(object_message, buffer);
  
  //Serial.print("Invio del messaggio al topic:");
  //Serial.println(topic);
  
  // topic name, retain = true, QoS = 1
  mqttClient.beginMessage(topic, true, 1); // crea nuovo messaggio da pubblicare
  
  // publish message
  mqttClient.print(buffer);
  mqttClient.endMessage();   
  
}

int letturaSogliaRaggiunta(){

  int sensore_profondita;
  int livello_raggiunto = 0;

  sensore_profondita = digitalRead(pin_S1);
  if(sensore_profondita==HIGH){
    livello_raggiunto = 1; // raggiunto livello 3 cm
  } 

  sensore_profondita = digitalRead(pin_S2);
  if(sensore_profondita==HIGH){
    livello_raggiunto = 2; // raggiunto livello 5 cm
  }

  sensore_profondita = digitalRead(pin_S3);
  if(sensore_profondita==HIGH){
    livello_raggiunto = 3; // raggiunto livello 10 cm
  }

  return livello_raggiunto;
  
}

// define an 'interrupt service routine' (ISR)
void MeterISR() {
    // let our flow meter count the pulses
    Meter->count();
}

void scanWifi(){

  // modalità di connessione ad una rete wifi
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  int n = WiFi.scanNetworks();
  if(n==0){
     Serial.println("Nessun network disponibile!");
  } else {
     for(int i=0; i<n; ++i){
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      if(WiFi.SSID(i)==ssid){
        Serial.println("ok trovato");
        flag_trovato = 1;
        setupWifi();
        break;
      }
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");
      delay(10);
     }
  }
  
}

int setupWifi(){

  WiFi.begin(ssid, password);
  Serial.println("Connessione alla rete wifi . .");

   while(WiFi.status() != WL_CONNECTED){
         Serial.print(".");
         delay(1000);
   }

  Serial.println("Connection established!");  
  digitalWrite(pinLedWifiStatus, HIGH);
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP()); 

  wifiClient.setCACert(ca_cert); // settaggio certificato 

  setupBrokerConnection();
  
  return 0;
 
}

String printLocalTime(){
  
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    return "";
  }
  char buffDate[50];
  strftime(buffDate, sizeof(buffDate), "%A, %B %d %Y %H:%M:%S", &timeinfo);
  String dateTimeAsString(buffDate);
  
  return dateTimeAsString;
  
}

float convertiValoreSensore(int value){

  float voltage, temperatura;
  
  //voltage = (value*3.3)/4095.0;    // 3.3v pin usage
  //voltage = (value*5)/4095.0;        // 5v pin usage
  voltage = (value/4095.0)*5.0;
  temperatura = ((voltage-0.5)*100)-10;   // temperatura in gradi
  

  return temperatura;
  
}
