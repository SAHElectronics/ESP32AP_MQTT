/*
 WiFi Web Server LED Blink

 A simple web server that lets you blink an LED via the web.
 This sketch will print the IP address of your ESP32 (once connected)
 to the Serial monitor. From there, you can open that address in a web browser
 to turn on and off the LED on pin 13.

 If the IP address of your shield is yourAddress:
 http://yourAddress/H turns the LED on
 http://yourAddress/L turns it off

 This example is written for a network using WPA2 encryption. For insecure
 WEP or WPA, change the Wifi.begin() call and use Wifi.setMinSecurity() accordingly.

 Circuit:
 * Adafruit Feather ESP32 V2

 created for arduino 25 Nov 2012
 by Tom Igoe

ported for sparkfun esp32 
31.01.2017 by Jan Hendrik Berlin

 modified to work with Adafruit Feather ESP32
 26.02.2023 by Sam Mastromatteo
 
 */

#include <WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

// wifi network
#define WLAN_SSID       "YOUR SSID HERE"
#define WLAN_PASS       "YOUR PASSWORD HERE"

#define LED 13

// Store the MQTT server, username, and password in flash memory.
// This is required for using the Adafruit MQTT library.
const char MQTT_SERVER[] PROGMEM    = "YOUR.SERVER.IP.HERE";
const char MQTT_USERNAME[] PROGMEM  = "";
const char MQTT_PASSWORD[] PROGMEM  = "";

const int mqttPort = 1883;

// If your MQTT Broker requires authentication
//const char* mqttUser = "";
//const char* mqttPassword = "";
const char* mqttTopic_sub = " YOUR_TOPIC_HERE";
//
/************ Global State (you don't need to change this!) ******************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// or... use WiFiFlientSecure for SSL
//WiFiClientSecure client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, MQTT_SERVER, mqttPort, MQTT_USERNAME, MQTT_PASSWORD);

/****************************** Feeds ***************************************/

// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Subscribe topic1 = Adafruit_MQTT_Subscribe(&mqtt, mqttTopic_sub);


/*************************** Sketch Code ************************************/

// Bug workaround for Arduino 1.6.6, it seems to need a function declaration
// for some reason (only affects ESP8266, likely an arduino-builder bug).
void MQTT_connect();

void setup() {
  pinMode(LED, OUTPUT);

  Serial.begin(115200);
  delay(10);

  Serial.println(F("Adafruit MQTT demo"));

  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());

  // Setup MQTT subscription for onoff & slider feed.
  mqtt.subscribe(&topic1);
  //mqtt.subscribe(&slider);
}

uint32_t x=0;

void loop() {
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  char *found;
  uint i;
  char *comData[10];

  MQTT_connect();

  // this is our 'wait for incoming subscription packets' busy subloop
  // try to spend your time here

  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(5000))) {
    // Check if its the onoff button feed
    if (subscription == &topic1) {
      Serial.print(F("Topic1: "));
      Serial.println((char *)topic1.lastread);
      
      found = strtok((char *)topic1.lastread,",");
      i = 0;
      while(found)
      {
        Serial.println(found);
        comData[i] = found;
        Serial.println(comData[i]);
        found = strtok(NULL,",");        
      }
      

      if (strcmp((char *)topic1.lastread, "w") == 0) {
        Serial.print("Forward\n"); 
      }
      if (strcmp((char *)topic1.lastread, "s") == 0) {
        Serial.print("Reverse\n");
      }
      if (strcmp((char *)topic1.lastread, "d") == 0) {
        Serial.print("Right\n"); 
      }
      if (strcmp((char *)topic1.lastread, "a") == 0) {
        Serial.println("Left");
      }
    }
  }

  // ping the server to keep the mqtt connection alive
  if(! mqtt.ping()) {
    mqtt.disconnect();
  }

}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}

