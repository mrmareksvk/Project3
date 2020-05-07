/*
    Embedded program of group Viziduc WireBark 2020 - Final 2nd. sem project

	Check readme.txt before working with this programe!

    GPIO16(WAKE) and RST has to be connected for proper function of this programe!
 */

// LIBRARIES
#include <ESP8266WiFi.h>
#include "Secret.h"                                 // Stores SSID , PSW
#include <DHT.h>                                    // TEM / HUM sensor
#include <PubSubClient.h>                           // MQTT
#include <TinyGPS++.h>                              // GPS
#include <SoftwareSerial.h>                         // GPIO Serial connection
// #include <CircularBuffer.h>                      // Better arrays

// DEFINITIONS
#define DHTPIN 13                                   // <-----
#define DHTTYPE DHT22                               // DHT22(AM2302)
#define LDR_PIN A0                                  // only analog pin on ESP=A0
#define IN_LED 2                                    // on-board LED on ESP=2
#define RXPinGPS 4                                  // <-----
#define TXPinGPS 5                                  // <-----
#define DEEP_SLEEP 10e6                             // in microseconds
#define MQTT_SERVER "broker.hivemq.com"             // <-----  LOCALHOST
#define MQTT_PORT 1883                              // <-----  default
#define ROOT_TOPIC "baaa"                           // Add '/' before device
#define DEVICE_NAME "/sensor1"                      // <-----  original w. '/'

// VARIABLES and CONSTANTS
float temperature, humidity;                        // raw dht data
uint16 light;                                       // light val max 1024
const uint16 light_treshold = 400;                  // minimal value to operate
bool dataToSend = false;                            // flag for data sending
bool maintenanceNeeded = false;                     // flag for maintenance
// TIMERS (49d. for millis roll-over)
unsigned long lastRecon = 0, timer1 = 0, lastReconnect = 0, interval = 5000;
// ARRAYS for MQTT values
char temp_a[6], hum_a[4], light_a[5];               // char buffers for publish
// -WORK-IN-PROGRESS
struct Storage {
	byte index;                                		// 1 byte
	float temp[25];                                 // 4 * 25 = 100 byte
	float hum[25];                                  // 4 * 25 = 100 byte
	uint16 light[25];                               // 2 * 25 = 50 byte
	double lat[25];                                 // 8 * 25 = 200 byte
	double lon[25];                                 // 8 * 25 = 200 byte
	uint32 date[25];                                // 4 * 25 = 100 byte
	uint32 time[25];                                // 4 * 25 = 100 byte
};                                                  // total == 901 byte
// GPS DATA VARIABLES
double latitude, longitude;
uint32 date_raw, time_raw;
char lat_a[10], lon_a[11];
char date_a[7], time_a[9];
// MQTT TOPIC PATHS
char temp_topic[20], hum_topic[20], light_topic[20];
char lat_topic[20], lon_topic[20];
char date_topic[20], time_topic[20];

// MQTT ASSIGN PATH FUNCTION
void pathAssign(void){
// temperature
	strcpy(temp_topic, ROOT_TOPIC);                     // Check readme.txt
	strcat(temp_topic, DEVICE_NAME);                    // Check readme.txt
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
bool measureTemp(void) {                            // <---------
	temperature = dht.readTemperature();
	/*
	   if isnan read last from buffer and copy it
	 */
	if(isnan(temperature)) {
		return false;
	}else{
		return true;
	}

}

bool measureHum(void) {                             // <----------
	humidity = dht.readHumidity();
	/*
	   if isnan read last from buffer and copy it
	 */
	if(isnan(humidity)) {
		return false;
	}else{
		return true;
	}

}

bool measureLight(void) {                           // <----------
	light = analogRead(LDR_PIN);
	/*
	   if isnan or not in interval 0 - 1024 read last from buffer and copy it
	 */
	if(isnan(light) || !(0 <= light && light <= 1024)) {
		return false;
	}else{
		return true;
	}
}

void gpsParser(void) {
	if (ss.available() > 0) {
		gps.encode(ss.read());

		if (gps.location.isUpdated()) {
			latitude = gps.location.lat();
			longitude = gps.location.lng();
		}

		if (gps.date.isUpdated()) {
			date_raw = gps.date.value();
		}

		if (gps.time.isUpdated()) {
			time_raw = gps.time.value();
		}

	}
}

unsigned long blink = 0;
void blinker(void){
	if(millis()>=blink+1000) {
		digitalWrite(IN_LED, !digitalRead(IN_LED));
		blink = millis();
	}
}

// Non-blocking mqtt connection function
boolean reconnect(void) {
	const char* clientId = DEVICE_NAME;
	if (mqttClient.connect(clientId, mqtt_user, mqtt_psw)) {
		sensorACK();
		if(dataToSend) {
			mqttPublish();
		}
	}
	return mqttClient.connected();
}

void mqttPublish(void) {
	dtostrf(temperature, 4, 1, temp_a);                 // Check readme.txt
	dtostrf(humidity, 3, 0, hum_a);
	sprintf(light_a, "%d", light);						// Check readme.txt
	dtostrf(latitude, 9, 6, lat_a);
	dtostrf(longitude, 10, 6, lon_a);
	sprintf(date_a, "%d", date_raw);                    // DDMMYY
	sprintf(time_a, "%d", time_raw);                    // HHMMSSCC
	time_a[strlen(time_a)-2] = '\0';                    // Removes centy-seconds

	mqttClient.publish(temp_topic, temp_a);
	mqttClient.publish(hum_topic, hum_a);
	mqttClient.publish(light_topic, light_a);
	mqttClient.publish(lat_topic, lat_a);
	mqttClient.publish(lon_topic, lon_a);
	mqttClient.publish(date_topic, date_a);
	mqttClient.publish(time_topic, time_a);

	dataToSend = false;
}

void sensorACK(void){									// ACK to ROOT_TOPIC
	char ack_msg[20];
	if(maintenanceNeeded) {
		strcpy(ack_msg, "maintenance");
		strcat(ack_msg, DEVICE_NAME);
		mqttClient.publish(ROOT_TOPIC, ack_msg);
	}else{
		strcpy(ack_msg, "working");
		strcat(ack_msg, DEVICE_NAME);
		mqttClient.publish(ROOT_TOPIC, ack_msg);
	}
}

void dataStorage(void){                             	// <----------!!!

}

void deepSleep(void){
	system_deep_sleep_set_option(2);                    // Check readme.txt
	system_deep_sleep(DEEP_SLEEP);
}

void setup() {
	// IO setup
	pinMode(IN_LED, OUTPUT);
	pinMode(LDR_PIN, INPUT);

	measureLight();
	if(light < light_treshold) {
		deepSleep();
	} else {
        WiFi.disconnect();                                  // Clears WiFi cache

		// INIT
		pathAssign();                                   // Init topic paths
        //Serial.begin(9600);                               // Only for DEBUG
		ss.begin(9600);                                 // GPS baudrate

		dht.begin();

		// WiFi SETUP
		WiFi.mode(WIFI_STA);                            // WiFi as station (NOT AP)
		WiFi.setSleepMode(WIFI_NONE_SLEEP);             // Full power, prevents random disconnects

		WiFi.begin(ssid, password);
		wifi_station_set_auto_connect(true);
		byte connectionAttemps = 0;
		while (WiFi.status() != WL_CONNECTED) {
			delay(200);
			digitalWrite(IN_LED, !digitalRead(IN_LED));
			connectionAttemps++;
			if (connectionAttemps >= 20) {
				break;                                          // Continue without network
			}
		}

		digitalWrite(IN_LED, HIGH);                     // Anti-logic

		mqttClient.setServer(MQTT_SERVER, MQTT_PORT);

		// data filler and maintenance invoker
		if (measureTemp() == false) {maintenanceNeeded=true;}
		if (measureHum() == false) {maintenanceNeeded=true;}
		if (measureLight() == false) {maintenanceNeeded=true;}

	}
}

void loop() {
	if(light >= light_treshold && maintenanceNeeded == false) {
		// M Q T T  H A N D L E R
		if (!mqttClient.connected()) {			// Loss of connection
			if (millis() >= lastReconnect + 2000) {   // reconnectInterval
				lastReconnect = millis();
				if (reconnect()) {         			// Try reconnect
					lastReconnect = 0;                                  // Succesfull reconection
				}
			}
		} else {                                            // Connected-keep MQTT alive
			mqttClient.loop();
		}

		// M E A S U R E M E N T S  5  M i n (TESTING SET FOR 5 SEC)
		if (millis() >= timer1 + interval) {
			measureTemp();
			measureHum();
			measureLight();

			if(WiFi.status() == WL_CONNECTED && mqttClient.connected()) {
				mqttPublish();
			}else{
				// dataStorage();                           // <----- ! ! !
				delay(10);
			}
			timer1 = millis();
		}

		gpsParser();                                        // Funnel data to GPS obj
		blinker();                                          // Debug

	}else{
		//sleep
		if(maintenanceNeeded == true) {
			sensorACK();
		}                                                   // Off we go
		mqttClient.publish(ROOT_TOPIC, "sleep we go");
		delay(100);
		deepSleep();
	}

}
