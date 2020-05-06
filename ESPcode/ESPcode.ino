/*TODO
    if newdata verification put outside from 5 minute loop
    every measure functions should have validation implemented
    if measurement failed copy last item in array and append it
    if connection. If storage array has 1 item use measurement array = same data

    if data and wifi: if storage>1 -upload storage, else upload measure array
    maybe do struct for whole data block
    add power naps
    non blocking wifi setup
    request maintenance to root topic

    struct
*/

// LIBRARIES
#include <ESP8266WiFi.h>
#include "Secret.h"                                 // Stores SSID , PSW
#include <DHT.h>
#include <PubSubClient.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <CircularBuffer.h>

// DEFINITIONS
#define ARR_SIZ(a) (sizeof(a) / sizeof(*a))         // maybe use
#define DHTPIN 13
#define DHTTYPE DHT22                               // DHT22(AM2302)
#define LDR_PIN A0
#define IN_LED 2
#define RXPinGPS 4                                  // <-----
#define TXPinGPS 5                                  // <-----
#define MQTT_SERVER "broker.hivemq.com"             // <----- LOCAL HOST
#define MQTT_PORT 1883
#define ROOT_TOPIC "baaa/"                          // Add / after
#define DEVICE_NAME "sensor1"

// VARIABLES
float temperature, humidity;
uint16 light;                                       // light val max 1024
bool dataToSend = false;                            // flag for data sending
bool maintenanceNeeded = false;                     // flag for maintenance
// Timers (49d. for millis roll-over)
unsigned long lastRecon = 0, timer1 = 0, lastReconnect = 0, interval = 5000;
// ARRAYS for MQTT values
char temp_a[6], hum_a[4], light_a[5];               // last received value

struct Storage {                                    // uint16(short) on light ?
    byte index;                                     // 1 byte
    float temp[25];                                 // 4 * 25 = 100 byte
    float hum[25];                                  // 4 * 25 = 100 byte
    uint16 light[25];                               // 2 * 25 = 50 byte
    double lat[25];                                 // 8 * 25 = 200 byte
    double lon[25];                                 // 8 * 25 = 200 byte
    uint32 date[25];                                // 4 * 25 = 100 byte
    uint32 time[25];                                // 4 * 25 = 100 byte
}                                                   // total == 901 byte

// LATITUDE 90.123456 / LONGITUDE 180.123456 (2/3 + 1(float point) + 6 + delim)
double latitude, longitude;
char lat_a[10], lon_a[11];
char date_a[7], time_a[9];
// MQTT TOPIC PATHS
char temp_topic[20], hum_topic[20], light_topic[20];
char lat_topic[20], lon_topic[20];
char date_topic[20], time_topic[20];

// MQTT ASSIGN PATH FUNCTION
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
DHT dht(DHTPIN, DHTTYPE);

// NETWORK SETUP
WiFiClient espClient;

/*Put your SSID & Password*/
const char* ssid = SECRET_SSID;                     //Secret.h
const char* password = SECRET_PSW;                  //Secret.h

// MQTT SETUP
PubSubClient mqttClient(espClient);

const char* mqtt_user = SECRET_MQTT_USER;
const char* mqtt_psw = SECRET_MQTT_PSW;

// FUNCTIONS
void measureTemp(void) {
  temperature = dht.readTemperature();
  if (isnan(temperature)){
      // check storage array
      // and copy last value
      return;
  }else{
  dtostrf(temperature, 4, 1, temp_a);               // MQTT: pass float to char array 4 long, 1 after dec. point
  }
}

void measureHum(void) {
  humidity = dht.readHumidity();
  if (isnan(humidity)){
      // check storage array
      //temp_a = temp_storage[];
      return;
  }else{
  dtostrf(humidity, 3, 0, hum_a);                   // check docu > dtostrf
  }
}

void measureLight(void) {                           // < do validity check
  light = analogRead(LDR_PIN);
//itoa(light, light_a, 10);                         // check docu > itoa
  sprintf(light_a, "%d", light);
}

// Non-blocking mqtt connection function
boolean reconnect(void) {
  const char* clientId = DEVICE_NAME;
  if (mqttClient.connect(clientId, mqtt_user, mqtt_psw)) {
      mqttClient.publish(ROOT_TOPIC, DEVICE_NAME);
      if(dataToSend){
          mqttPublish();
      }
  }
  return mqttClient.connected();
}

void mqttPublish(void) {
  mqttClient.publish(temp_topic, temp_a);
  mqttClient.publish(hum_topic, hum_a);
  mqttClient.publish(light_topic, light_a);
  mqttClient.publish(lat_topic, lat_a);
  mqttClient.publish(lon_topic, lon_a);
  mqttClient.publish(date_topic, date_a);
  mqttClient.publish(time_topic, time_a);
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

    if (gps.date.isUpdated()){
        // final array,string "%d", value
        sprintf(date_a, "%d", gps.date.value());    // DDMMYY
    }

    if (gps.time.isUpdated()){
        // final array,string "%d", value
        sprintf(time_a, "%d", gps.time.value());    // HHMMSSCC
        time_a[strlen(time_a)-2] = '\0';            // remove centy-seconds
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
  Serial.begin(9600);                               // only for DEBUG
  ss.begin(9600);                                   // GPS baudrate

  dht.begin();

  // WiFi SETUP
  WiFi.mode(WIFI_STA);                              // WiFi works as station (NOT AP)
  WiFi.setSleepMode(WIFI_NONE_SLEEP);               // Full power, prevents random disconnects

  WiFi.begin(ssid, password);
  wifi_station_set_auto_connect(true);
  byte connectionAttemps = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    digitalWrite(IN_LED, !digitalRead(IN_LED));
    connectionAttemps++;
    if (connectionAttemps >= 20){
        break;                                      // Continue without network
    }
  }

  digitalWrite(IN_LED, HIGH);                       // anti-logic

  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);

}

void loop() {
  // -----------------  M Q T T  H A N D L E R
  if (!mqttClient.connected()) {
  // Loss of connection
    if (millis() >= lastReconnect + 2000) {     // reconnectInterval
      lastReconnect = millis();
      // Try reconnect
      if (reconnect()) {
        // Succesfull reconection
        lastReconnect = 0;
      }
    }
  } else {
    // connected, keep calling for mqtt
    mqttClient.loop();
  }

  // -----------------  M E A S U R E M E N T S  5  M i n
  // testing set for 5 seconds
  if (millis() >= timer1 + interval) {
    timer1 = millis();
    measureTemp();
    measureHum();
    measureLight();
    dataToSend = true;
    mqttPublish();
  }

  gpsParser();

}
