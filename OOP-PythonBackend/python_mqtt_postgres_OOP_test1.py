import sensor_mqtt
import sensor_db

sensor = sensor_mqtt.Sensor_MQTT()
sensor.setTopic("baaa/#")
sensor.setLogin("pi", "123")
sensor.connectMQTT("192.168.0.192")

database = sensor_db.Sensor_DB("127.0.0.1", "ooptest", "postgres", "admin")

while True:
    sensor.loop()
    if sensor.isAvailable():
        database.saveData(*sensor.fetchMessage())
