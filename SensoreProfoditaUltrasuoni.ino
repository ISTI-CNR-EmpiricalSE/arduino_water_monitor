/*
 * 03/05/2022
 * Programma sensore di profondità
 * @rosario_parisi
 */
#include <LiquidCrystal_I2C.h>
#define PROVA

/* Altezze posizione sensori */
const int altezza_1 = 3;  // 3 cm
const int altezza_2 = 5;  // 5 cm
const int altezza_3 = 10; // 10 cm

/* Digital pin to receive signal from water sensor */
const int pin_S1 = 2;
const int pin_S2= 3;
const int pin_S3 = 4;

/* Variables to capture distance from Ultrasonic Sensor*/
int distance_cm_captured = 0;
int distance_cm_from_sensor = 40; // cm
int value_profondita = 0;

/* Variable to capture water level from analog_read */
int livello_raggiunto;
int livello_raggiunto_prec = 0;

/* Setup display1 e display2 (address, num char, numr rows) */
LiquidCrystal_I2C lcd_1(0x27,16,2);
LiquidCrystal_I2C lcd_2(0x25,16,2);

long readUltrasonicDistance(int triggerPin, int echoPin){

  pinMode(triggerPin, OUTPUT);
  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);

  // sets trigger pin to HIGH state for 10 microsec
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerPin, LOW);
  pinMode(echoPin, INPUT);

  // conta il tempo che impiega l'impulso per andare e ritornare
  return pulseIn(echoPin, HIGH); 
   
}

int letturaSogliaRaggiunta(){

  int sensore_profondita;
  
  sensore_profondita = digitalRead(pin_S3);
  if(sensore_profondita==HIGH){
    return 3; // raggiunto livello 10 cm
  }
  sensore_profondita = digitalRead(pin_S2);
  if(sensore_profondita==HIGH){
    return 2; // raggiunto livello 5 cm
  }
  sensore_profondita = digitalRead(pin_S1);
  if(sensore_profondita==HIGH){
    return 1; // raggiunto livello 3 cm
  } 

  return 0; // nessun livello raggiunto
  
}

void updateLcdDisplay(int livello_raggiunto, LiquidCrystal_I2C lcd){

  //lcd.clear();
  //lcd.setCursor(5,1);
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

void setup() {
  
  Serial.begin(9600);
  pinMode(pin_S1, INPUT);
  pinMode(pin_S2, INPUT);
  pinMode(pin_S3, INPUT);

  /* Setting Display1 */
  lcd_1.begin();
  lcd_1.backlight();
  lcd_1.clear();
  lcd_1.setCursor(0,0);
  lcd_1.print("Liv.Raggiunto:");
  lcd_1.setCursor(0,1);
  lcd_1.print("Altezza cm:0");
 

  /* Setting Display2 */
  lcd_2.begin();
  lcd_2.backlight();
  lcd_2.clear();
  lcd_2.setCursor(0,0);
  lcd_2.print("**Lev.Measure***");
  lcd_2.setCursor(0,1);
  lcd_2.print("** SYS 2022  ***");

  // effettuo lettura del pin del sensore
  livello_raggiunto = letturaSogliaRaggiunta();
  updateLcdDisplay(livello_raggiunto, lcd_1);
  
  
}

void loop() {

   /*
    * Velocità del suono nelle nostre condizioni a 20°C è di 343,1 m/s (1235, 16km/h)
    * Conversione da m/s >> cm/microsecondi 0,03431 cm/microsecondi
    * 
    * Spazio percorso = velocità x tempo
    * Spazio (cm) -- 0,03431 * t (microsecondi)
    * L'impulso deve andare e tornare, quindi divido per 2 =>
    * Spazio = 0,017155 * t
    */
  
  // misura ping time in cm
  #ifndef PROVA
  distance_cm_captured = 0.01723 * readUltrasonicDistance(7, 7);
  Serial.print("Distance in cm");
  Serial.print(distance_cm_captured);
  
  /*
   * valore profondità = distance_cm_from_sensor - distance_cm_captured
   * In questo caso il sensore ultrasuoni può considerare oggetti solo per una
   * distanza massima di 30 cm
   * 
   */
   // calcolo profondità raggiunta (precisa)
   value_profondita = distance_cm_from_sensor-distance_cm_captured;
   #endif
  
   // effettuo lettura del pin del sensore
   livello_raggiunto = letturaSogliaRaggiunta();

   // mando in output il livello raggiunto
   Serial.print("Raggiunto livello: ");
   Serial.println(livello_raggiunto);

   // update LCD display
   if(livello_raggiunto_prec!=livello_raggiunto){
      updateLcdDisplay(livello_raggiunto, lcd_1);
   }
   
   livello_raggiunto_prec = livello_raggiunto;

   // wait 100ms before next reading
   delay(100);

}
