#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>


#define INLED 2             // <------------
#define RXPin 0             // <------------
#define TXPin 4            // <------------

long timer=0;
double longitude, latitude;
String date_string, datestamp, timestamp, time_string;


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
  server.send(200, "text/html", SendHTML(longitude, latitude, date_string, time_string));
  delay(500);
}

void handle_NotFound() {
  server.send(404, "text/plain", "Not found");
}

String SendHTML(double templongitude, double templatitude, String datestamp, String timestamp) {
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
  char latitude[11];
  dtostrf(templatitude, 8, 6, latitude);
  ptr += latitude;
  ptr += "</p>";
  ptr += "<p>Longitude: ";
  char longitude[11];
  dtostrf(templongitude, 8, 6, longitude);
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
  

  //IP: WiFi.localIP()

  // Handlers
  server.on("/", handle_OnConnect);
  server.onNotFound(handle_NotFound);

  server.begin();

}
void loop(){
  // This sketch displays information every time a new sentence is correctly encoded.
  server.handleClient();
  while (ss.available() > 0){
    gps.encode(ss.read());
    if (gps.location.isUpdated()){
      latitude = gps.location.lat();
      longitude = gps.location.lng();
//      datestamp = gps.date.value();
      if (gps.date.isValid()){
        datestamp = gps.date.month();
        datestamp +="/";
        datestamp += gps.date.day();
        datestamp +="/";
        datestamp += gps.date.year();
      }
      date_string = datestamp;
      datestamp = "";
      Serial.println(datestamp);
//      timestamp = gps.time.value();
      if (gps.time.isValid()){
        timestamp = gps.time.hour();
        timestamp += ":";
        timestamp += gps.time.minute();
        timestamp += ":";
        timestamp += gps.time.second();
        }
       time_string = timestamp;
       timestamp = "";
//      Serial.println(timestamp);
//      Serial.println(latitude);

    }
  }
}
