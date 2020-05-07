ReadMe for ESPcode.ino
Viziduc WireBark 2020

- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
PREDISPOSITIONS
    • Make sure Secret.h file is in the same folder, this is its structure
        #define SECRET_SSID "";
        #define SECRET_PSW "";
        #define SECRET_MQTT_USER "";
        #define SECRET_MQTT_PSW "";
    • Bridge GPIO 16 with RST > !THIS IS CRUCIAL TO MAKE DEEP SLEEP WORK!
    • Make sure // DEFINITIONS are correctly filled
    • Don't use long MQTT Topic paths! It is draining memory. Default is 20 characters
    • Make sure DEVICE_NAME is original for every device, to prevent possible conflicts in MQTT brooker (though it is very unlikely, you was warned!)
    • Make sure there is slash '/' in front of DEVICE NAME!

LIBRARIES
    • <ESP8266WiFi.h>
    • <DHT.h>
    • <PubSubClient.h>
    • <TinyGPS++.h>
    • <SoftwareSerial.h>
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

GENERAL POWER DRAW
    • 120mA when fully operational
    • 40mA when deep sleeping

GENERAL PROGRAM FUNCTIONS:
    • dtostrf(floatvar, StringLengthIncDecimalPoint, numVarsAfterDecimal, charbuf);
    • strcpy(to, from); - copies and add delimiter
    • strcat(to, from); - appends by overriding delimiter and adding new one
    • char *  itoa ( int value, char * str, int base ); - !NOT USED!
    • dtostrf(); > function converts double to char array (avr-libc)
    • sprintf(); > formated output to char array (string)

MQTT:
     MQTT topic for every sensor
        •	/baaa/sensorX/temp
        •	/baaa/sensorX/hum
        •	/baaa/sensorX/lux
        •	/baaa/sensorX/lat
        •	/baaa/sensorX/lon
        •	/baaa/sensorX/date
        •	/baaa/sensorX/time

     MQTT Main topic for every sensor
        •   /baaa
        devices sends acknowledge here

TinyGPS++:
        To get TinyGPS++ to work, you have to repeatedly funnel the characters to it from the GPS module using the encode() method.

IMPROVEMENTS:
    Calibrate LDR to the LUX values. Use datasheet > GL5537
    https://www.kth.se/social/files/54ef17dbf27654753f437c56/GL5537.pdf

    Turn of sensors with transistors to draw less power when sleeping
    GPS module takes ~ 30mA

OTHER:
     c_types.h -ESP8266 - - - - - - - - - - - - - - - - - - - - - - -

     typedef signed char         sint8_t;
     typedef signed short        sint16_t;
     typedef signed long         sint32_t;
     typedef signed long long    sint64_t;
     typedef unsigned long long  u_int64_t;
     typedef float               real32_t;
     typedef double              real64_t;

     typedef unsigned char       uint8;
     typedef unsigned char       u8;
     typedef signed char         sint8;
     typedef signed char         int8;
     typedef signed char         s8;
     typedef unsigned short      uint16;
     typedef unsigned short      u16;
     typedef signed short        sint16;
     typedef signed short        s16;
     typedef unsigned int        uint32;
     typedef unsigned int        u_int;
     typedef unsigned int        u32;
     typedef signed int          sint32;
     typedef signed int          s32;
     typedef int                 int32;
     typedef signed long long    sint64;
     typedef unsigned long long  uint64;
     typedef unsigned long long  u64;
     typedef float               real32;
     typedef double              real64;

     CircularBuffer - - - - - - - - - - - - - - - - - - - - - - - - -
     https://github.com/rlogiacco/CircularBuffer

     buffer.unshift() 	- add item to begining
     buffer.push()		- add item to the end

     buffer.shift()		- take item from begining and delete item
     buffer.pop()		- take item from end and delete item

     buffer.first()		- copy first
     buffer.last()		- copy last

     buffer.isEmpty()	- bool
     buffer.isFull()	- bool
     buffer.size()		- how much in buffer right now
     buffer.available()	- how much free
     buffer.capacity()	- total size of buffer (how much can store)
     buffer.clear()		- reset whole buffer

     DEEP_SLEEP - - - - - - - - - - - - - - - - - - - - - - - - - - -
     https://www.espressif.com/sites/default/files/9b-esp8266-low_power_solutions_en_0.pdf

     deep_sleep_set_option(0)
     The 108th Byte of init parameter decides whether RF calibration will
     be performed after the chip wakes up from Deep-sleep.

     deep_sleep_set_option(1)
     The chip will make RF calibration after waking up from Deep-sleep.
     • Power consumption is high.

     deep_sleep_set_option(2)
     The chip won’t make RF calibration after waking up from Deep-sleep.
     • Power consumption is low.

     deep_sleep_set_option(4)
     The chip won’t turn on RF after waking up from Deep-sleep.
     • Power consumption is the lowest, same as in Modem-sleep

     LIGHT-SLEEP: - - - - - - - - - - - - - - - - - - - - - - - - - -
        3.7. Force Sleep APIs in ESP8266 NON-OS SDK API Reference p.69

        wifi_set_sleep_type(LIGHT_SLEEP_T);
        Auto Light-sleep, ESP8266 automatically enters Light-sleep mode when connected to Wi-Fi with the CPU idle.

     PubSubClient.h - - - - - - - - - - - - - - - - - - - - - - - - -
     https://pubsubclient.knolleary.net/

     int publish (topic, payload) {const char[]; const char[]}
     int publish (topic, payload, length) {const char[]; byte[]; byte}
     int publish (topic, payload, retained) {const char[]; const char[]; bool}

     ESP: - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
     https://www.espressif.com/sites/default/files/documentation/2c-esp8266_non_os_sdk_api_reference_en.pdf
