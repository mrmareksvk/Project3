import sensor_mqtt
import sensor_db_v2

sensor = sensor_mqtt.Sensor_MQTT()
sensor.setTopic("baaa/#")
sensor.setLogin("pi", "123")
sensor.connectMQTT("192.168.0.192")

database = sensor_db_v2.Sensor_DB()
database.setDB("ooptest")
database.setLogin("postgres", "admin")
database.connectDB("127.0.0.1")

while True:
    sensor.loop()
    if sensor.isAvailable():
        database.saveData(*sensor.fetchMessage())
