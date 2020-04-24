/*  POKEC and TODO:
     WiFi.mode(WIFI_NONE_SLEEP) ✔
     WiFi.setSleepMode(WIFI_NONE_SLEEP);

*/

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DHT.h>
#include <PubSubClient.h>
#include "Secret.h" //SSID , PSW

#define DHTPIN 13
#define DHTTYPE DHT22
#define LDR_PIN A0
#define IN_LED 2
#define RELAY_PIN 4 // 4 je rezistorovana
#define GREEN_LED 12
#define BLUE_LED 13
#define RED_LED 15

float temperature, humidity;
char temp_a[4], hum_a[4], relay_a[1] = {0}; // ARRAYS for MQTT
String temp_s = "", hum_s = "", light_s = "", relay_s = "";
long lastReconnectTime, timer1 = 0, timer2 = 0;
int light, interval = 1000, sending_interval = 10000;

//DHT OBJECT
DHT dht22(DHTPIN, DHTTYPE);

/*Put your SSID & Password*/
const char* ssid = SECRET_SSID; //Secret.h
const char* password = SECRET_PSW; //Secret.h

//SERVER OBJECT
ESP8266WebServer server(80);

//NETWORK CLIENT OBJECT
WiFiClient espClient;

//MQTT
const char* mqtt_server = "test.mosquitto.org";

PubSubClient mqttClient(espClient);

//WEB
char web[] PROGMEM = R"====(
<!DOCTYPE html>
<html>
<head>
<meta name='viewport' content='width=device-width, initial-scale=1.0, user-scalable=no'>
<script src="https://kit.fontawesome.com/028dec8df7.js" crossorigin="anonymous"></script>
<script>
  //setTimeout
  setInterval(function(){ getTemp(); },5000);
  setInterval(function(){ getHum(); },5000);
  setInterval(function(){ getLight(); },2000);
  //setInterval(function(){ getRec(); },1500);
  //setInterval(function(){ getVar(); },1000);

  function getTemp() {
    console.log("Temp");
    var xTemp = new XMLHttpRequest();
    xTemp.onreadystatechange = function()
    {
      if (this.readyState == 4 && this.status == 200) {
        document.getElementById("temp").innerHTML = this.responseText;
        }
    };
    xTemp.open("GET", "/temp", true);
    xTemp.send();
    };

  function getHum() {
    console.log("Hum");
    var xHum = new XMLHttpRequest();
    xHum.onreadystatechange = function()
    {
      if (this.readyState == 4 && this.status == 200) {
        document.getElementById("hum").innerHTML = this.responseText;
        }
    };
    xHum.open("GET", "/hum", true);
    xHum.send();
    };

  function getLight() {
    console.log("Ligh");
    var xLight = new XMLHttpRequest();
    xLight.onreadystatechange = function()
    {
      if (this.readyState == 4 && this.status == 200) {
        document.getElementById("light").innerHTML = this.responseText;
        }
    };
    xLight.open("GET", "/light", true);
    xLight.send();
    };

</script>
<title>ESP8266 Control</title>
<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}
  body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;}
  p{font-size: 24px;color: #444444;margin-bottom: 10px;}
</style>
</head>
<body>
<div id='main'>
  <h1>ESP8266 Room control panel</h1>
  <hr>
  <p><i class="fas fa-thermometer-three-quarters" style="color:#059e8a;"></i>
  Temperature: <span id='temp'>--</span>&deg;C</p>
  <p><i class="fas fa-tint"></i>
  Humidity: <span id='hum'>--</span>%</p>
  <p><i class="fas fa-sun"></i>
  Luminity: <span id='light'>--</span></p>
  <p><i class="fas fa-power-off"></i>
  Relay state: <span id='rel'>--</span></p>
  <br>
  <button onclick='buttoner()'>Toggle relay</button>
  <br>
  <!-- p>Received: <span id='rec'>--</span></p> -->
  <!-- <p>Var: <span id='var'>--</span></p> -->
</div>
</body>
<script>
function buttoner(){
  console.log('Relay button');
  var xRelay = new XMLHttpRequest();

  xRelay.onreadystatechange = function(){
    if (this.readyState == 4 && this.status == 200){
      document.getElementById('rel').innerHTML = this.responseText;
      }
    };
    xRelay.open("GET", "/relay", true);
    xRelay.send();
  };
  document.addEventListener("DOMContentLoaded", buttoner, false);
</script>
</html>
)====";


void measureTemp(void){
  temperature = dht22.readTemperature();
  dtostrf(temperature, 4, 1, temp_a);       // MQTT: pass float to char array 4 long, 1 after dec. point
  temp_s = String(temperature);             // WEB: pass float to String for WebServer
  }

void measureHum(void){
  humidity = dht22.readHumidity();
  dtostrf(humidity, 4, 1, hum_a);
  hum_s = String(humidity);
  }

void measureLight(void){
  light = analogRead(LDR_PIN);
  light_s = String(light);
  }

void getTemp(void){
  measureTemp();
  server.send(200, "text/plain", temp_s);
  }

void getHum(void){
  measureHum();
  server.send(200, "text/plain", hum_s);
  }

void getLight(void){
  measureLight();
  server.send(200, "text/plain", light_s);
  }

 void switchRelay(void){
  if(digitalRead(RELAY_PIN) != relay_a[0]){
    digitalWrite(RELAY_PIN, relay_a[0]);
    } else {
    digitalWrite(RELAY_PIN, !digitalRead(RELAY_PIN)); //CHANGE RELAY STATE
    relay_a[0] = digitalRead(RELAY_PIN);
    }
  }

// ON / OFF by button # not automatization implemented; auto ajax call pre buttoner
void getRelayState(void){
  switchRelay();
  relay_s = digitalRead(RELAY_PIN) ? "ON" : "OFF"; // IF digital read true "ON" else "OFF"
  mqttClient.publish("mrmrk/rel", relay_a, sizeof(relay_a));
  server.send(200,"text/plain", relay_s); // RETURN ITS VALUE
  }

void mqttWeather(void){
  measureTemp();
  mqttClient.publish("mrmrk/temp", temp_a, sizeof(temp_a)); // publish takes arrays
  delay(100);
  measureHum();
  mqttClient.publish("mrmrk/hum", hum_a, sizeof(hum_a));
  }

void onConnect(void){
  server.send_P(200,"text/plain", web);
  }

void notFound(void){
  server.send(404, "text/plain", "Not found");
  }

String rec_s = "";
void callback(char* topic, byte* payload, unsigned int length) {
  if( (char)payload[0] == '0' || (char)payload[0] == '1'){ //  || (char)payload[0] == '2'
    relay_a[0] = char(payload[0]);
    digitalWrite(IN_LED, LOW);
    getRelayState();
  }else {
    digitalWrite(IN_LED, HIGH);
    }

  rec_s = "";
  for (int i = 0; i < length; i++) {
      rec_s += char(payload[i]);
  }
}

void getVar(void){
  server.send(200, "text/plain", relay_a);
  }

void getRec(void) {
  server.send(200, "text/plain", rec_s);
}


boolean reconnect(void) {
  String clientId = "WittyClient-";
  clientId += String(random(0xffff), HEX);
  if (mqttClient.connect(clientId.c_str())) {
    mqttClient.publish("mrmrk", "reconnected");
    delay(50);
    mqttClient.publish("mrmrk/temp", temp_a, sizeof(temp_a));
    delay(50);
    mqttClient.publish("mrmrk/hum", hum_a, sizeof(hum_a));
    delay(50);
    mqttClient.publish("mrmrk/rel", relay_a, sizeof(relay_a));
    delay(50);
    mqttClient.subscribe("mrmrk");
  }
  return mqttClient.connected();
}

void setup() {
  //WiFi.disconnect();
  //delay(100);

  //pinMode(RED_LED, OUTPUT);
  pinMode(IN_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LDR_PIN, INPUT);

  dht22.begin();

  // SETUP WiFi
  WiFi.mode(WIFI_STA);  // WiFi works as station (NOT AP)
  WiFi.setSleepMode(WIFI_NONE_SLEEP);  // Full power, prevents random disconnects

  WiFi.begin(ssid, password);
  wifi_station_set_auto_connect(true);
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    digitalWrite(IN_LED, !digitalRead(IN_LED));
  }

  mqttClient.setServer(mqtt_server, 1883);
  mqttClient.setCallback(callback);
  lastReconnectTime = 0;

  // HANDLERS
  server.on("/",[](){server.send_P(200, "text/html", web);});
  server.on("/temp", getTemp);
  server.on("/hum", getHum);
  server.on("/light", getLight);
  server.on("/relay", getRelayState);
  //server.on("/rec", getRec);
  //server.on("/var", getVar);
  server.onNotFound(notFound);

  server.begin();
}

void loop() {
  server.handleClient();
  // -----------------  M Q T T  C O N N E C T I O N  -----------------
  if (!mqttClient.connected()) {
    // signalize loss of connection by LED
    digitalWrite(GREEN_LED, HIGH);
    if (millis() >= lastReconnectTime + 2000){
      lastReconnectTime = millis();
      // Try reconnect
      if (reconnect()) {
        // succesfull reconection, turn off LED
        digitalWrite(GREEN_LED, LOW);
        lastReconnectTime = 0;
        }
      }
  } else {
    // connected, keep looping (scanning)
    mqttClient.loop();
    }

  // -----------------  M Q T T  S E N D I N G  1 0 S e c  -----------------
  if (millis() >= timer2 + sending_interval) {
    mqttWeather();
    timer2 = millis();
  }

}


  // ----------------- ! S I K R I T  S T A S H  D N T  T C H ! -----------------
  /*
  function getRec() {
    console.log("Rec");
    var xRec = new XMLHttpRequest();
    xRec.onreadystatechange = function()
    {
      if (this.readyState == 4 && this.status == 200) {
        document.getElementById("rec").innerHTML = this.responseText;
        }
    };
    xRec.open("GET", "/rec", true);
    xRec.send();
    };

    function getVar() {
    console.log("Var");
    var xVar = new XMLHttpRequest();
    xVar.onreadystatechange = function()
    {
      if (this.readyState == 4 && this.status == 200) {
        document.getElementById("var").innerHTML = this.responseText;
        }
    };
    xVar.open("GET", "/var", true);
    xVar.send();
    };
    */
