import paho.mqtt.client as mqtt


class Sensor_MQTT:
    def __init__(self, MQTTtopic="", MQTTusername="", MQTTpassword=""):

        self.__mqttTopic = MQTTtopic
        self.__mqttUsername = MQTTusername
        self.__mqttPassword = MQTTpassword
        self.__mqttClient = mqtt.Client()

        self.__messageSensorID = ""
        self.__messageTemperature = ""
        self.__messageHumidity = ""
        self.__messageLux = ""
        self.__messageLatitude = ""
        self.__messageLongitude = ""
        self.__messageDate = ""
        self.__messageTime = ""

        self.__messageCompiled = False

    def setTopic(self, topic):
        self.__mqttTopic = topic

    def setLogin(self, username, password):
        self.__mqttUsername = username
        self.__mqttPassword = password

    def connectMQTT(self, host="127.0.0.1", port=1883, keepalive=60):
        if self.__mqttTopic != "":
            self.__mqttClient.on_connect = self.__subscribeMQTT

        if self.__mqttUsername != "":
            self.__mqttClient.username_pw_set(self.__mqttUsername, self.__mqttPassword)

        self.__mqttClient.on_message = self.__processMessage

        self.__mqttClient.connect(host, port, keepalive)

    def __subscribeMQTT(self, client, userdata, flags, rc):
        self.__mqttClient.subscribe(self.__mqttTopic)

    def __processMessage(self, client, userdata, msg):
        if self.__messageSensorID == "":
            self.__messageSensorID = msg.topic[11]
        if "temp" in msg.topic:
            self.__messageTemperature = msg.payload.decode("utf8")
        elif "hum" in msg.topic:
            self.__messageHumidity = msg.payload.decode("utf8")
        elif "lux" in msg.topic:
            self.__messageLux = msg.payload.decode("utf8")
        elif "lat" in msg.topic:
            self.__messageLatitude = msg.payload.decode("utf8")
        elif "lon" in msg.topic:
            self.__messageLongitude = msg.payload.decode("utf8")
        elif "date" in msg.topic:
            self.__messageDate = msg.payload.decode("utf8")
            if len(self.__messageDate) == 5:
                self.__messageDate = "0" + self.__messageDate
            self.__messageDate = (
                "20" + self.__messageDate[4:]
                + "-" + self.__messageDate[2:4]
                + "-" + self.__messageDate[:2]
            )
        elif "time" in msg.topic:
            self.__messageTime = msg.payload.decode("utf8")
            if len(self.__messageTime) == 5:
                self.__messageTime = "0" + self.__messageTime
            self.__messageTime = (
                self.__messageTime[:2]
                + ":" + self.__messageTime[2:4]
                + ":" + self.__messageTime[4:]
            )

        if "" not in {
            self.__messageSensorID,
            self.__messageTemperature,
            self.__messageHumidity,
            self.__messageLux,
            self.__messageLatitude,
            self.__messageLongitude,
            self.__messageDate,
            self.__messageTime
        }:
            self.__messageCompiled = True

    def isAvailable(self):
        return self.__messageCompiled

    def fetchMessage(self):
        if self.__messageCompiled is True:
            self.__fullMessage = (
                self.__messageSensorID,
                self.__messageTemperature,
                self.__messageHumidity,
                self.__messageLux,
                self.__messageLatitude,
                self.__messageLongitude,
                self.__messageDate,
                self.__messageTime
            )

            self.__messageSensorID = ""
            self.__messageTemperature = ""
            self.__messageHumidity = ""
            self.__messageLux = ""
            self.__messageLatitude = ""
            self.__messageLongitude = ""
            self.__messageDate = ""
            self.__messageTime = ""

            self.__messageCompiled = False

            return self.__fullMessage

    def loop(self):
        self.__mqttClient.loop()
