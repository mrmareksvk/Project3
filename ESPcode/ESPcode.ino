// LIBRARIES
#include <ESP8266WiFi.h>
#include "Secret.h"                                 // Stores SSID , PSW
#include <DHT.h>
#include <PubSubClient.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>

// DEFINITIONS                                      // <---------- !!!
#define DHTPIN 13
#define DHTTYPE DHT22                               // DHT22(AM2302)
#define LDR_PIN A0
#define IN_LED 2
#define RXPinGPS 3
#define TXPinGPS 1
#define ROOT_TOPIC "baaa"                           // <---------- issue 1
#define DEVICE_NAME "sensor1"

// VARIABLES
float temperature, humidity;
// ARRAYS for MQTT values
char temp_a[4], hum_a[4], light_a[4];
double latitude, longitude;
char lat_a[10], lon_a[11];                          // 6 demical precis + delim
char temp_topic[20], hum_topic[20], light_topic[20];
char lat_topic[20], lon_topic[20];
char date_topic[20], time_topic[20];
int light;                                          // light val max 1024
bool dataToSend = false;                            // flag for data sending
// Timers (49d. millis roll)
unsigned long lastRecon = 0, timer1 = 0, lastReconnect = 0, interval = 300000;

// MQTT TOPIC PATHS
void pathAssign(void){
// temperature
strcpy(temp_topic, ROOT_TOPIC);                     // copies and add delimiter
strcat(temp_topic, DEVICE_NAME);                    // appends by override delim
strcat(temp_topic, "/temp");
// humidity
strcpy(hum_topic, ROOT_TOPIC);
strcat(hum_topic, DEVICE_NAME);
strcat(hum_topic, "/hum");
// light
strcpy(light_topic, ROOT_TOPIC);
strcat(light_topic, DEVICE_NAME);
strcat(light_topic, "/lux");
// latitude
strcpy(lat_topic, ROOT_TOPIC);
strcat(lat_topic, DEVICE_NAME);
strcat(lat_topic, "/lat");
// longitude
strcpy(lon_topic, ROOT_TOPIC);
strcat(lon_topic, DEVICE_NAME);
strcat(lon_topic, "/lon");
// GPS date
strcpy(date_topic, ROOT_TOPIC);
strcat(date_topic, DEVICE_NAME);
strcat(date_topic, "/date");
// GPS time
strcpy(time_topic, ROOT_TOPIC);
strcat(time_topic, DEVICE_NAME);
strcat(time_topic, "/time");
}

// INITIALIZATIONS

// The TinyGPS++ object
TinyGPSPlus gps;

// The serial connection to the GPS device
SoftwareSerial ss(RXPinGPS, TXPinGPS);

// DHT OBJECT
DHT dht22(DHTPIN, DHTTYPE);

// NETWORK SETUP
WiFiClient espClient;

/*Put your SSID & Password*/
const char* ssid = SECRET_SSID;                     //Secret.h
const char* password = SECRET_PSW;                  //Secret.h

// MQTT setup
const char* mqtt_server = "192.168.155.128";        // <----------!
const int mqtt_port = 1883;

PubSubClient mqttClient(espClient);

// FUNCTIONS
void measureTemp(void) {
  temperature = dht22.readTemperature();
  dtostrf(temperature, 4, 1, temp_a);               // MQTT: pass float to char array 4 long, 1 after dec. point
}

void measureHum(void) {
  humidity = dht22.readHumidity();
  dtostrf(humidity, 4, 1, hum_a);
}

void measureLight(void) {
  light = analogRead(LDR_PIN);
}

// Non-blocking mqtt connection function
boolean reconnect(void) {                           // <------- !!
  String clientId = DEVICE_NAME;
  if (mqttClient.connect(clientId.c_str()) && dataToSend) {
    mqttPublish();
  }
  return mqttClient.connected();
}

void mqttPublish(void) {                            // remove delays?
  mqttClient.publish(temp_topic, "test");
  //delay(25);
  mqttClient.publish(hum_topic, "test");
  //delay(25);
  mqttClient.publish(light_topic, "test");
  //delay(25);
  mqttClient.publish(lat_topic, "test");
  //delay(25);
  mqttClient.publish(lon_topic, "test");
  //delay(25);
  mqttClient.publish(date_topic, "test");
  //delay(25);
  mqttClient.publish(time_topic, "test");
  //delay(25);
  dataToSend = false;
}

void gpsParser(void) {
  if (ss.available() > 0) {
    gps.encode(ss.read());
    if (gps.location.isUpdated()) {
        latitude = gps.location.lat();
        dtostrf(latitude, 9, 6, lat_a);

        longitude = gps.location.lng();
        dtostrf(longitude, 10, 6, lon_a);

    }
  }

}

void setup() {
  WiFi.disconnect();                                // clears WiFi cache
  // delay(50);

  // IO setup
  pinMode(IN_LED, OUTPUT);
  pinMode(LDR_PIN, INPUT);

  // INIT
  pathAssign();                                     // init topic paths
//Serial.begin(9600);                               // only for DEBUG
  ss.begin(9600);                                   // GPS baudrate

  dht22.begin();

  // WiFi SETUP
  WiFi.mode(WIFI_STA);                              // WiFi works as station (NOT AP)
  WiFi.setSleepMode(WIFI_NONE_SLEEP);               // Full power, prevents random disconnects

  WiFi.begin(ssid, password);
  wifi_station_set_auto_connect(true);
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    digitalWrite(IN_LED, !digitalRead(IN_LED));
  }

  digitalWrite(IN_LED, LOW);                        // maybe anti logic LOW=HIGH

  mqttClient.setServer(mqtt_server, mqtt_port);


}

void loop() {
  // -----------------  M Q T T  C O N N E C T I O N  -----------------
  if (!mqttClient.connected()) {
    // TODO: Change LEDs for debug prints?
    // signalize loss of connection by LED
    digitalWrite(IN_LED, HIGH);
    if (millis() >= lastReconnect + 2000) {     // + reconnectInterval
      lastReconnect = millis();
      // Try reconnect
      if (reconnect()) {
        // succesfull reconection, turn off LED
        digitalWrite(IN_LED, LOW);
        lastReconnect = 0;
      }
    }
  } else {
    // connected, keep calling for mqtt
    mqttClient.loop();
  }

  // -----------------  M Q T T  S E N D I N G  5  M i n  -----------------
  if (millis() >= timer1 + interval) {
    // TODO  -  functions, check integrity from every function
    timer1 = millis();
    dataToSend = true;
  }
}
