# arduino_water_monitor 
## Overview
Arduino Water Monitor è un sistema di misurazione del flusso di un corso d'acqua e del livello di profondità. Può essere impiegato per la prevenzione di allagamenti o per il semplice monitoraggio relativo ad oggetti di osservazione. I dati raccolti dal sensore vengono inviati ad un topic dedicato attraverso un broker MQTT e il client Telegraf, iscritto al topic, li rende disponbili nel database di tipo time-stamp InfluxDB. Il microcontrollore usato è Arduino UNO, ma può essere compatibile anche con ESP32 tramite l'IDE di Arduino.

## Usage

## Installation

1. Installa Arduino IDE a questo [link](https://www.arduino.cc/en/software)

2. Installa InfluxDB 2.20:
   - Mac OS from Homebrew
      > * brew update
      > * brew install influxdb
   - Mac OS manual version [qui](https://portal.influxdata.com/downloads/)
   - Windows, Linux version [qui](https://portal.influxdata.com/downloads/)
   
3. Installa Telegraf:
   - Mac OS from Homebrew
      > * brew update
      > * brew install telegraf
   - Mac OS manual version [qui](https://portal.influxdata.com/downloads/)
   - Windows, Linux version [qui](https://portal.influxdata.com/downloads/)


## Additional Installation

Delle librerie addizionali rispetto a quelle fornite dell'IDE di Arduino sono necessarie:
1. ArduinoJson (Arduino Ide -> strumenti -> gestione librerie -> cerca ArduinoJson -> installa)
2. FlowMeter: [download](https://github.com/sekdiy/FlowMeter)
    *  crea la directory "FlowMeter" in Documenti/Arduino/libraries
    *  inserisci il file scaricato al suo interno 
3. LiquidCrystal_I2C: [download](https://github.com/fdebrabander/Arduino-LiquidCrystal-I2C-library)
    *  crea la directory "LiquidCrystal_I2C" in Documenti/Arduino/libraries
    *  inserisci il file scaricato al suo interno 
5. WifiClientSecure: (Arduino Ide -> strumenti -> Scheda -> gestore scheda -> esp32)
6. ArduinoMqttClient (Arduino Ide -> strumenti -> gestione librerie -> cerca ArduinoClientMqtt -> installa)

# First Use

## InfluxDB Configuration (from dashboard)

1. Dopo aver installato InfluxDB, accedi a localhost:8086
2. Crea Username e Password
3. Accedi alla dashboard di Influx
4. Crea un bucket: "your_bucket_name" e.g: "arduinowatermonitor"
5. Data > Api Tokens > Generate Token Key > All Access API token

## Telegraf Configuration (from cmd)

### Configure Output InfluxDB 

1. Vai alla cartella: /etc/telegraf
2. Modifica i seguenti parametri del file di telegraf.conf: come mostrato in figura

<img width="698" alt="config telegraf" src="https://user-images.githubusercontent.com/60266562/169335862-265b50fe-0d67-4d7d-9049-6fae3fdabf79.png">

3. Adesso hai configurato il bucket (del database) dove le informazioni del sensore vengono pubblicate

### Configure Input from MQTT topic sensor
1. Sempre nel file di configurazione: telegraf.conf
2. Modifica i seguenti parametri: indirizzo del tuo server broker, nome del topic, username, password

<img width="634" alt="config-telegraf-mqttconsumer" src="https://user-images.githubusercontent.com/60266562/169343405-f72b591e-8f8a-40ba-9f06-fe02683fa0ef.png">



