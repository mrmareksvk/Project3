#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>

#define INLED 16             // <------------
#define RXPin 0             // <------------
#define TXPin 2            // <------------

long timer=0;
double longitude, latitude, datestamp, timestamp;

/*Put your SSID & Password*/
const char* ssid = "Vartic1";               // Enter SSID here
const char* password = "DaTaCaGaSe";           // Enter Password here


// server object
ESP8266WebServer server(80);

// The TinyGPS++ object
TinyGPSPlus gps;

// The serial connection to the GPS device
SoftwareSerial ss(RXPin, TXPin);

void handle_OnConnect() {
  server.send(200, "text/html", SendHTML(longitude, latitude, datestamp, timestamp));
  delay(500);
}

void handle_NotFound() {
  server.send(404, "text/plain", "Not found");
}

String SendHTML(double longitude, double latitude, double datestamp, double timestamp) {
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";

  ptr += "<script>\n";
  ptr += "setInterval(loadDoc,5000);\n";
  ptr += "function loadDoc() {\n";
  ptr += "var xhttp = new XMLHttpRequest();\n";
  ptr += "xhttp.onreadystatechange = function() {\n";
  ptr += "if (this.readyState == 4 && this.status == 200) {\n";
  ptr += "document.body.innerHTML =this.responseText}\n";
  ptr += "};\n";
  ptr += "xhttp.open(\"GET\", \"/\", true);\n";
  ptr += "xhttp.send();\n";
  ptr += "}\n";
  ptr += "</script>\n";

  ptr += "<title>ESP8266 GPS</title>\n";
  ptr += "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr += "body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;}\n";
  ptr += "p {font-size: 24px;color: #444444;margin-bottom: 10px;}\n";
  ptr += "</style>\n";
  ptr += "</head>\n";
  ptr += "<body>\n";
  ptr += "<div id=\"webpage\">\n";
  ptr += "<h1>ESP8266 GPS testing</h1>\n";
  ptr += "<p>Latitude: ";
  ptr += latitude;
  ptr += "</p>";
  ptr += "<p>Longitude: ";
  ptr += longitude;
  ptr += "</p>";
  ptr += "<p>Date: ";
  ptr += datestamp;
  ptr += "<p>Time ";
  ptr += timestamp;
  ptr += "</p><br>";
  ptr += "</div>\n";
  ptr += "</body>\n";
  ptr += "</html>\n";
  return ptr;
}
void http_get() {
  server.send(200, "text/plain", "This request works");
}


void setup() {
  ss.begin(9600);     // GPS baudrate https://www.u-blox.com/sites/default/files/NEO-M8_DataSheet_(UBX-13003366).pdf
  Serial.begin(9600);
  WiFi.disconnect();
  delay(100);
  pinMode(INLED, OUTPUT);
  Serial.println("setup");
  //begin wi-fi connection
  WiFi.begin(ssid, password);

  //check wi-fi if is connected yet
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    digitalWrite(INLED, !digitalRead(INLED));
  }
  Serial.print(WiFi.localIP());

  while (ss.available() > 0) {
    gps.encode(ss.read());
    if (gps.location.isUpdated()) {
      latitude = gps.location.lat();
      longitude = gps.location.lng();
      datestamp = gps.date.value();       // Raw date in DDMMYY format (u32)
      timestamp = gps.time.value();       // Raw time in HHMMSSCC format (u32)
    }
  }

  //IP: WiFi.localIP()

  // Handlers
  server.on("/", handle_OnConnect);
  server.onNotFound(handle_NotFound);

  server.begin();

}
void loop() {
  server.handleClient();
  if (millis() >= timer + 5000) {
    timer = millis();
    Serial.println("timer");
    while (ss.available() > 0) {
//      Serial.println("whileloop");
      gps.encode(ss.read());
      if (gps.location.isUpdated()) {
        latitude = gps.location.lat();
        longitude = gps.location.lng();
        datestamp = gps.date.value();
        timestamp = gps.time.value();
//        Serial.println(latitude);
//        Serial.println(longitude);
//        Serial.println(datestamp);
//        Serial.println(timestamp);
      }
    }
  }


}
