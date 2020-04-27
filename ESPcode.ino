/*  TODO and stuff:
     WiFi.mode(WIFI_NONE_SLEEP) ✔
     WiFi.setSleepMode(WIFI_NONE_SLEEP);
     -
     ARRAYS FOR GPS
     ARRAYS FOR KEEPING DATA

     last measurement checker
     if light measure
     light > change to lux?

     GPS function
     WiFi reconnect function
     REWORK WIFI SETUP
*/

// LIBRARIES
#include <ESP8266WiFi.h>
#include <DHT.h>
#include <PubSubClient.h>
#include "Secret.h"                                 //SSID , PSW

// DEFINITIONS
#define DHTPIN 13
#define DHTTYPE DHT22                               // DHT11 / DHT22(AM2302)
#define LDR_PIN A0
#define IN_LED 2

// VARIABLES
float temperature, humidity;                        // FLOATS Measurements
char temp_a[4], hum_a[4], light_a[4];               // ARRAYS for MQTT
long lastRecon = 0, timer1 = 0, interval = 300000;  // TIMERS
int light,                                          //INT LIGHT 1024

// INITIALIZATIONS

// DHT OBJECT
DHT dht22(DHTPIN, DHTTYPE);

// NETWORK SETUP
WiFiClient espClient;
/*Put your SSID & Password*/
const char* ssid = SECRET_SSID;                     //Secret.h
const char* password = SECRET_PSW;                  //Secret.h

// MQTT setup
const char* mqtt_server = "test.mosquitto.org";

PubSubClient mqttClient(espClient);

// FUNCTIONS
void measureTemp(void){
  temperature = dht22.readTemperature();
  dtostrf(temperature, 4, 1, temp_a);               // MQTT: pass float to char array 4 long, 1 after dec. point
  }

void measureHum(void){
  humidity = dht22.readHumidity();
  dtostrf(humidity, 4, 1, hum_a);
  }

void measureLight(void){
  light = analogRead(LDR_PIN);
  }

// NON-BLOCKING MQTT FUNCTION
boolean reconnect(void) {
  String clientId = "WireSharkViziduc";
  // clientId += String(random(0xffff), HEX);
  if (mqttClient.connect(clientId.c_str())) {
    mqttClient.publish("*topic*", "*string*");                  // <-------
    delay(50);
    mqttClient.publish("*topic*", temp_a, sizeof(temp_a));      // <-------
    delay(50);
    mqttClient.publish("*topic*", hum_a, sizeof(hum_a));        // <-------
    delay(50);
    mqttClient.publish("*topic*", light_a, sizeof(relay_a));    // <-------
    delay(50);
  }
  return mqttClient.connected();
}

void setup() {
  //WiFi.disconnect();
  //delay(100);

  // IO setup
  pinMode(IN_LED, OUTPUT);
  pinMode(LDR_PIN, INPUT);

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

  mqttClient.setServer(mqtt_server, 1883);
  lastReconnectTime = 0;

}

void loop() {
  // -----------------  M Q T T  C O N N E C T I O N  -----------------
  if (!mqttClient.connected()) {
    // signalize loss of connection by LED
    digitalWrite(IN_LED, HIGH);
    if (millis() >= lastReconnectTime + 2000){
      lastReconnectTime = millis();
      // Try reconnect
      if (reconnect()) {
        // succesfull reconection, turn off LED
        digitalWrite(IN_LED, LOW);
        lastReconnectTime = 0;
        }
      }
  } else {
    // connected, keep looping (scanning)
    mqttClient.loop();
    }

  // -----------------  M Q T T  S E N D I N G  5  M i n  -----------------
  if (millis() >= timer2 + interval) {
    // TODO  -  functions
    timer1 = millis();
  }
}


  // ----------------- ! S I K R I T  S T A S H  D N T  T C H ! ------------
