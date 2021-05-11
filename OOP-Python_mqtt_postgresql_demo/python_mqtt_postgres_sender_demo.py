import paho.mqtt.client as mqtt


def on_connect(client, userdata, flags, rc):
    print("Connected with result code " + str(rc))


client = mqtt.Client()
client.on_connect = on_connect

client.username_pw_set("pi", "123")
client.connect("192.168.0.192", 1883, 60)

while True:
    if input() == "send":
        print("sending shit")
        client.publish("baaa/sensor1/temp", "25.5")
        client.publish("baaa/sensor1/hum", "35")
        client.publish("baaa/sensor1/lux", "500")
        client.publish("baaa/sensor1/lat", "56.164881")
        client.publish("baaa/sensor1/lon", "10.181980")
        client.publish("baaa/sensor1/date", "2020-04-30")
        client.publish("baaa/sensor1/time", "20:22:01")
    client.loop()
