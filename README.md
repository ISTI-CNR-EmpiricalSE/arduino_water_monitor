# arduino_water_monitor 
## Overview
Arduino Water Monitor è un sistema di misurazione del flusso di un corso d'acqua e del livello di profondità. Può essere impiegato per la prevenzione di allagamenti o per il semplice monitoraggio relativo ad oggetti di osservazione. I dati raccolti dal sensore vengono inviati ad un topic dedicato attraverso un broker MQTT e il client Telegraf, iscritto al topic, li rende disponbili nel database di tipo time-stamp InfluxDB. Il microcontrollore usato è Arduino UNO, ma può essere compatibile anche con ESP32 tramite l'IDE di Arduino.

## Versioni

### Arduino UNO

Puoi trovare diverse versioni del progetto:
1) Arduino Sketch + Script python + Sensore Acqua YS-F201 (branch/master)
   * lo sketch di arduino si occupa di effettuare le letture dei dati e codificare i messaggi in formato json
   * tramite la porta seriale sono inviati allo script python
   * lo script agisce da publisher nel topic MQTT dedicato al sensore
   * il messaggio json è della forma: {"CurFlowRate":0,"TotVolumeRate":0,"Long":43.71,"Lat":10.42}

2) Arduino Sketch con Display LCD I2C + sensore profondità
   * Il sensore profondità si occupa di misurare la profondità dell'acqua su 3 livelli differenti:
      - livello 1: 3cm
      - livello 2: 5cm
      - livello 3: 10cm
   * Le informazioni sono stampate nel display LCD (16x2) 
<img width="438" alt="Schermata 2022-05-20 alle 11 57 17" src="https://user-images.githubusercontent.com/60266562/169504316-a20389eb-8e48-49da-959e-918934d6c0de.png">

## Dashboard InfluxDB example:

<img width="1632" alt="influxDashboard" src="https://user-images.githubusercontent.com/60266562/174997759-139656b4-ab44-44ef-96e9-0d9b69045849.png">


### esp32

 1) Arduino Sketch con Display LCD + Sensore profondità + Sensore ultrasuoni + Sensore Acqua YS-F201 (Telegraf&InfluxDB usage) [master/Esp32MonitorBox]
   * Le informazioni raccolte dal sensore riguardano
      - la profondità dell'acqua
      - il flusso dell'acqua
      - temperatura dell'acqua
      - temperatura esterna
      - luminosità ambientale
   * lo sketch arduino invia le informazioni dei sensori in maniera diretta al topic MQTT 
   * il client Telegraf si occupa di leggere i messaggi del topic MQTT e di registrarli nel database Influxdb (guarda la guida sotto per l'installazione e configurazione)

## Usage

* ⛰️ Sensore a valle o a monte per prevenire rischio idreogeologico
* ⛲ Misurazione flusso d'acqua
* 〰️ Misurazione profondità acqua
* :high_brightness: Luminosità ambientale
* :thermometer: Temperatura ambiente e acqua
* 💧 Piccoli home-project

<img width="511" alt="monitor seriale prova" src="https://user-images.githubusercontent.com/60266562/169485351-92b4cf81-6f6a-4303-90f6-2d6a52364ecf.png">

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
1. ArduinoJson (6.19.3): (Arduino Ide -> strumenti -> gestione librerie -> cerca ArduinoJson -> installa)
2. FlowMeter (1.2.0): [download](https://github.com/sekdiy/FlowMeter)
    *  crea la directory "FlowMeter" in Documenti/Arduino/libraries
    *  inserisci il file scaricato al suo interno 
3. LiquidCrystal_I2 (2017 version)C: [download](https://github.com/fdebrabander/Arduino-LiquidCrystal-I2C-library)
    *  crea la directory "LiquidCrystal_I2C" in Documenti/Arduino/libraries
    *  inserisci il file scaricato al suo interno 
 
5. WifiClientSecure: (Arduino Ide -> strumenti -> Scheda -> gestore scheda -> esp32 (2.0.3) )
    - Andare su Arduino/Preferenze e inserisci il seguente URL: https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
    - Selezionare la scheda: Arduino Ide -> strumenti -> scheda -> ESP32 Arduino -> DOIT ESP32 DEVKIT V1
7. ArduinoMqttClient (0.1.5): (Arduino Ide -> strumenti -> gestione librerie -> cerca ArduinoMqttClient -> installa)
8. DallasTemperature (3.9.0): (Arduino Ide -> strumenti -> gestione librerie -> cerca DallasTemperature -> installa)
9. (nota: Quando il prompt chiede se installare la dipendeza da OneWire, rispondere installa soltanto DallasTemperature)
10. OneWire: [updated version](https://github.com/arendst/Tasmota/tree/development/lib/lib_basic/OneWire-Stickbreaker) 
11. (nota: Scaricare l'interno file zip dalla cartella root Tasmota, lib/lib_basice/OneWire-StickBreaker, copiare il contenuto all'interno della cartellaOneWire creata da te in Arduino/libreries)

# First Use

## InfluxDB Configuration (from dashboard)

esegui 
  >  influxd run 
1. Dopo aver installato InfluxDB, accedi a localhost:8086
2. Crea Username,Password e scegli il nome dell'organizzazione
3. Accedi alla dashboard di Influx
4. Crea un bucket: "your_bucket_name" e.g: "arduinowatermonitor"
5. Data > Tokens > Generate Token Key > All Access API token 

## Telegraf Configuration (from cmd)

### Configure Output InfluxDB 

1. Cerca il file: telegraf.conf e aprilo per la modifica
2. All'interno della sezione OUTPUT PLUGINS cerca [[outputs.influxdb_v2]]
3. Decommentare urls = ["http://127.0.0.1:8086"]
4. Modifica i seguenti parametri del file di telegraf.conf: inserisci il token creato in precedenza al passo (5), nome dell'organizzazione scelta durante la fase (2) e nome del bucket scelto al passo (4)

<img width="698" alt="config telegraf" src="https://user-images.githubusercontent.com/60266562/169335862-265b50fe-0d67-4d7d-9049-6fae3fdabf79.png">

3. Adesso hai configurato il bucket (del database) dove le informazioni del sensore vengono pubblicate

### Configure Input from MQTT topic sensor

1. Sempre nel file di configurazione: telegraf.conf
2. Modifica i seguenti parametri: indirizzo del tuo server broker (dovrebbe iniziare con mqtt://) , nome del topic, username e password server broker
3. Se non hai un server broker creato in precedenza scarica MQTT explorer e crea una MQTT Connection nuova
<img width="634" alt="config-telegraf-mqttconsumer" src="https://user-images.githubusercontent.com/60266562/169343405-f72b591e-8f8a-40ba-9f06-fe02683fa0ef.png">

### Adesso sei pronto per iniziare...

1. Scarica lo sketch: (uno dei file .ino della cartella master)
2. Alcuni parametri sono da modificare: il nome della reta (ssid), la password, l'indirizzo del broker, username_broker
pwd_broker. Il parametro your_certificate deve essere prelevato dal server MQTT creato da te in precedenza, copia-incollalo tra BEGIN-CERTIFICATE e END-CERTIFICATE
3. Avvia l'IDE di Arduino
4. Collega la porta seriale al Mac/Pc
5. Sezione: strumenti/porta/dev.cu<port_name> dove <port_name> è il nome della porta associata alla seriale dal tuo computer.
6. Effettuare upload dello sketch sulla scheda (freccia in alto a sinistra)
7. Selezioneare il monitor seriale e impostarlo a (115200 bound)
8. Dovresti vedere "Done uploading", allora lo sketch è caricato sulla scheda
9. Puoi visualizzare cosa succede durante l'esecuzione dal monitor seriale

### Avvia telegraf e influxdb (from cmd)

> * influxd run (terminale)
> * telegraf --config  /etc/telegraf/telegraf.conf (in un altro terminale) 
(il percorso telegraf.conf, può variare assicurarsi di inserire quello corretto)


