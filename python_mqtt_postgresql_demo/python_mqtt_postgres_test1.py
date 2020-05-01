import paho.mqtt.client as mqtt
import psycopg2
from psycopg2.extensions import ISOLATION_LEVEL_AUTOCOMMIT
import msvcrt

# Creates DB if it doesn't exist

con = psycopg2.connect(
    host="127.0.0.1", database="postgres", user="postgres", password="admin"
)
con.set_isolation_level(ISOLATION_LEVEL_AUTOCOMMIT)

cur = con.cursor()

cur.execute("SELECT 1 FROM pg_catalog.pg_database WHERE datname = 'sensortest'")
exists = cur.fetchone()
if not exists:
    cur.execute("CREATE DATABASE sensortest")

cur.close()
con.close()

##################

# Connects to database and creates tables for 5 devices

con = psycopg2.connect(
    host="127.0.0.1", database="sensortest", user="postgres", password="admin"
)

cur = con.cursor()

for tableID in range(1, 6):
    cur.execute(
        "CREATE TABLE IF NOT EXISTS sensor%s ("
        "   id_entry SERIAL PRIMARY KEY,"
        "   temperature DOUBLE PRECISION,"
        "   humidity INT,"
        "   lux INT,"
        "   latitude DOUBLE PRECISION,"
        "   longitude DOUBLE PRECISION,"
        "   dt TIMESTAMP"
        ")",
        (tableID,),
    )

con.commit()

##################


# ID, temp, hum, lux, lat, lon, date, time
sensorDATA = [-1] * 8


def on_connect(client, userdata, flags, rc):
    print("Connected with result code " + str(rc))
    client.subscribe("baaa/#")


def on_message(client, userdata, msg):
    global sensorDATA
    if sensorDATA[0] == -1:
        sensorID = msg.topic[11]
        sensorDATA[0] = sensorID
    # elif is more effective
    if "temp" in msg.topic:
        sensorDATA[1] = msg.payload.decode("utf8")
    if "hum" in msg.topic:
        sensorDATA[2] = msg.payload.decode("utf8")
    if "lux" in msg.topic:
        sensorDATA[3] = msg.payload.decode("utf8")
    if "lat" in msg.topic:
        sensorDATA[4] = msg.payload.decode("utf8")
    if "lon" in msg.topic:
        sensorDATA[5] = msg.payload.decode("utf8")
    if "date" in msg.topic:
        sensorDATA[6] = msg.payload.decode("utf8")
    if "time" in msg.topic:
        sensorDATA[7] = msg.payload.decode("utf8")


client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.username_pw_set("pi", "123")
client.connect("192.168.0.192", 1883, 60)

while True:
    if -1 not in sensorDATA:
        sensorDATA[6] = (
            "20" + sensorDATA[6][4:]
            + "-" + sensorDATA[6][2:4]
            + "-" + sensorDATA[6][:2]
        )
        sensorDATA[7] = (
            sensorDATA[7][:2] + ":" + sensorDATA[7][2:4] + ":" + sensorDATA[7][4:6]
        )

        print("saving shit")
        cur.execute(
            "INSERT INTO sensor%s (temperature, humidity, lux, latitude, longitude, dt) VALUES(%s, %s, %s, %s, %s, %s)",
            (
                int(sensorDATA[0]),
                sensorDATA[1],
                sensorDATA[2],
                sensorDATA[3],
                sensorDATA[4],
                sensorDATA[5],
                sensorDATA[6] + " " + sensorDATA[7],
            ),
        )
        con.commit()
        sensorDATA = [-1] * 8
    client.loop()

    # press q to exit
    if msvcrt.kbhit():
        if msvcrt.getch() == b"q":
            cur.close()
            con.close()
            break
