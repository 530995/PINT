from paho.mqtt import client as mqtt
import pyodbc
import schedule
import time

port = 1883
broker = "test.mosquitto.org"
topic = "topic/temperature"

DRIVER_NAME = 'ODBC Driver 17 for SQL Server'
SERVER_NAME = 'DESKTOP-VFKLOHS\MSSQLSERVER01'
DATABASE_NAME = 'GY-251'

cnxn_str = f"""DRIVER={{{DRIVER_NAME}}};
            SERVER={SERVER_NAME};
            DATABASE={DATABASE_NAME};
            Trusted_Connection=yes;
"""

def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Connected successfully")
        client.subscribe(topic)
    else:
        print(f"Connection failed with code {rc}")

def on_message(client, userdata, message):

     print(f"Received {int(message.payload.decode())} from {message.topic} topic")

     try:
          conn = pyodbc.connect(cnxn_str)
     except Exception as e:
          print(e)
          print('task is terminated')
     else:
          cursor = conn.cursor()

     insert_statement = f"""INSERT INTO MPU(Temperature) VALUES ({int(message.payload.decode())})"""

     try:
          print(insert_statement)
          cursor.execute(insert_statement)
     except Exception as e:
          cursor.rollback()
          print(e)
          print('transaction rolled back')
     else:
          print('records inserted successfully')
          cursor.commit()
          cursor.close()
     finally:
          conn.closed

def run():

     client = mqtt.Client()

     client.on_connect = on_connect
     client.on_message = on_message
                    
     client.connect(broker, port)

     startTime = time.time()
     waitTime = 0.15
     while True:
          client.loop()
          elapsedTime = time.time() - startTime
          if elapsedTime > waitTime:
               client.disconnect()
               break

schedule.every(1).minutes.do(run)

while 1:
     schedule.run_pending()
     time.sleep(1)