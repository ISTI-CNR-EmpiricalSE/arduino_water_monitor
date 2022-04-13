import random
import signal
import sys
import threading
import time
import certifi
import serial
import json

from paho.mqtt import client as mqtt_client

"""
    Subst yours parameters in accord to your broker host, port, username and pwd
    Subst arduino serial port name 
"""

arduino = serial.Serial("/dev/cu.usbmodem14101", 9600)
arduino.timeout = 1

#broker = 'your broker host'
#port = 'your broker port'
#topic = "python/mqtt"
#client_id = f'python-mqtt-{random.randint(0,1000)}'
#username = 'usernmae'
#password = 'password'

topicRead = 'python/commandToArduino'
topicSensorYFS201 = 'python/topicYFS201'

class threadPublisher(threading.Thread):

    """
        A class used to represent Client Pusblisher Thread

        Attributes:
        ----------
        threadId: int
            number that represent the progressive number of the thread
        name: str
            name of the thread
        client : str
            a string that represent the client id during connection to Broker

        Methods:
        --------
        run()
            it starts thread execution

    """

    def __init__(self, threadID, name, client):
        threading.Thread.__init__(self)
        self.threadID = threadID
        self.name = name
        self.client = client

    def run(self):
        print("Stard thread publisher " + self.name)
        # funzione del thread publisher
        run_publisher(self.client)
        print("Exit from thread" + self.name)

class threadSubscriber(threading.Thread):
    """
        A class used to represent Client Subscriber Thread

        Attributes:
        ----------
        threadId: int
            number that represent the progressive number of the thread
        name: str
            name of the thread
        client : str
            a string that represent the client id during connection to Broker

        Methods:
        --------
        run()
            it starts thread execution

    """

    def __init__(self, threadID, name, client):
        threading.Thread.__init__(self)
        self.threadID = threadID
        self.name = name
        self.client = client

    def run(self):
        print("Stard thread subscriber " + self.name)
        # funzione del thread subscriber
        run_subscriber(self.client)
        print("Exit from thread" + self.name)

def readSerialMessagesFromArduino():
    """ Read and return the message reading from Arduino Serial
    :return: message
    """
    message = arduino.readline().decode("ascii")
    if message=="":
        return
    return message

def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Connected to MQTT Broker!")
    else:
        print("Failed to connect, return code %d\n", rc)

def connect_mqtt(id_client) -> mqtt_client:
    """ Connect the client to the MQTT Broker
    :param id_client:
    :return: client connection
    """

    # settaggio connessione al server
    client = mqtt_client.Client(id_client)
    client.username_pw_set(username, password)

    #settaggio certificato
    client.tls_set(certifi.where())
    client.tls_insecure_set(False)

    # registra la funzione di callback on_connect
    client.on_connect = on_connect

    # connessione al server
    client.connect(broker, port)

    return client

def publish(client):
    """ Publish a message to the topic of the sensor YFS201

    :param: client
    :return: -

    :raises: KeyboardInterrupt
        If SIQINT is received from keyboard
    """
    counter_msg = 0

    try:
        while True:
            time.sleep(5)
            #msg = f"messages: {counter_msg}"

            # lettura seriale dal monitor arduino
            msg = readSerialMessagesFromArduino()
            res = client.publish(topicSensorYFS201, msg)

            counter_msg += 1
    except KeyboardInterrupt:
        print("Fine")


def on_message(client, userdata, msg):

     msg_received = msg.payload.decode()
     print(f"Received " + msg_received)

     # encoding messaggio ricevuto
     command_bytes = msg_received.encode()

     if msg_received=="stop":
         arduino.close()
         arduino.write("") # lancia un eccezione e la comunicazione si chiude
     else:
         # invio bytes nella porta seriale
         data_command = {}

         if msg_received=="accendi":
            #string_command_json = '{"led":"1"}'
            data_command["led"] = "1"
         else:
            #string_command_json = '{"led": "0"}'
            data_command["led"] = "0"

         #--------metodo con stringa costruita manualmente
         #data = string_command_json
         #print(data)
         #arduino.write(bytes(data.encode('ascii')))
         #arduino.flush()

         #--------metodo con uso json dumps
         data = json.dumps(data_command)
         print(data)
         arduino.write(data.encode('ascii'))
         arduino.flush()

def subscribe(client: mqtt_client):
    """ Subscribe client to Command to Arduino topic

    :param client:
    :return: -
    """
    print(client.subscribe(topicRead))
    client.on_message = on_message

def run_publisher(client):
    """ Function used from thread to start publishing

    :param client: broker connection
    :return: -
    """

    try:
     client.loop_start()
     publish(client)
    except: KeyboardInterrupt
    print("Fine pubblicazione Topic")
    client.loop_stop()
    arduino.close()

def run_subscriber(client):
    """ Function used from thread to start subscribing reading topic

    :param client: broker connection
    :return:
    """

    subscribe(client)
    client.loop_forever()

def signal_handler(signal, frame):
    print("exit from program")
    sys.exit(0)

# -------------------------| main function |-------------------------- #
if __name__ == '__main__':

    # set signal SIGINT handler
    signal.signal(signal.SIGINT, signal_handler)

    client_publisher = connect_mqtt(f'python-mqtt-client_publisher')
    client_subscriber = connect_mqtt(f'python-mqtt-client_subscriber')

    thread_publisher = threadPublisher(1, "Thread-Publisher-1", client_publisher)
    thread_subscriber = threadSubscriber(2, "Thread-Subscriber-2", client_subscriber)

    thread_publisher.daemon = True
    thread_subscriber.daemon = True

    # starting threads
    thread_publisher.start()
    thread_subscriber.start()

    thread_publisher.join()
    thread_subscriber.join()