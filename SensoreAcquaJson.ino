#include <FlowMeter.h>
#include <ArduinoJson.h>

const unsigned long eventIntervalSensorFlow = 1000;   // event interval to handle sensor reading
const unsigned long eventIntervalSendValues = 10000;  // event interval to handle values sending
const unsigned long eventIntervalBlinkLed = 500;      // event interval to handle led blinking
unsigned long previusMillis1 = 0;                     // previus MS sensor reading
unsigned long previusMillis2 = 0;                     // previus MS values sending
unsigned long previusMillis3 = 0;                     // previus MS led blinking
  

/*  capacity - l/min
 *  kFactor - "kFactor" in (pulse/s) l/min  -- Pulse Frequency = 7.5 * L/min
 *  mFactor[10] - multiplicative correction factor
 */
 
FlowMeter *Meter;
FlowSensorProperties YFS201 = {30.0f, 2.25f, {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}};
const String sensor1 = "WflowSensValle";
const String sensor2 = "WflowSensMonte";
const unsigned long period = 1000;
const int pinLed = 13;

// define an 'interrupt service routine' (ISR)
void MeterISR() {
    // let our flow meter count the pulses
    Meter->count();
}

void setup() {

  Serial.begin(9600);

  // setup led Blinking
  pinMode(pinLed, OUTPUT);
  digitalWrite(pinLed,LOW);

  // Sensor1 YF-S201
  Meter = new FlowMeter(digitalPinToInterrupt(2), YFS201, MeterISR, RISING);

}

void jsonMessageToSend(){

  StaticJsonDocument<200> doc; // alloca nello stack 200bytes di memoria
  JsonObject object_message = doc.to<JsonObject>();
  
  object_message["CurFlowRate"] = Meter->getCurrentFlowrate();
  object_message["TotVolumeRate"] = Meter->getTotalVolume();
  object_message["Long"] = 43.71;
  object_message["Lat"] = 10.42;
  
  serializeJson(object_message, Serial);
  Serial.println();
  
}

void jsonReadMessageInput(){

  String payload;
  //while( !Serial.available() ){}

  if(Serial.available()){
    
    payload = Serial.readStringUntil( '\n' );
    StaticJsonDocument<256> doc;

    //Serial.print((char)Serial.read());
    
    DeserializationError error = deserializeJson(doc, payload);
    if(error){
      Serial.print("Errore");
      Serial.println(error.c_str());
      return;
    }

    if(doc["led"] == "1"){
      // accendo il led
      digitalWrite(pinLed, HIGH);
      Serial.print("accendo");
    } else {
      // spengo il led
      digitalWrite(pinLed, LOW);
      Serial.print("spengo");
    }
    
  }
  delay(20);
  
}


void loop() {

  // current milliseconds
  unsigned long nowMillis = millis();

  // every 1 second read from water flow sensor (to have data up to date)
  if( nowMillis - previusMillis1 > eventIntervalSensorFlow ){

    Meter->tick(period);
    previusMillis1 = nowMillis;
    
  } 

  // every 10 seconds sends values to MQTT python publisher
  if( nowMillis - previusMillis2 > eventIntervalSendValues ){

    jsonMessageToSend();
    previusMillis2 = nowMillis;
    
  }

  // every 500 millisenconds read values from MQTT subscriber
  if( nowMillis - previusMillis3 > eventIntervalBlinkLed ){
    
    jsonReadMessageInput();
    previusMillis3 = nowMillis;
    
  }
  
}
