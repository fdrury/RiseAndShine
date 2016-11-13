/***************************************************
  Adafruit MQTT Library ESP8266 Example

  Must use ESP8266 Arduino from:
    https://github.com/esp8266/Arduino

  Works great with Adafruit's Huzzah ESP board:
  ----> https://www.adafruit.com/product/2471
  Adafruit invests theTime and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Tony DiCola for Adafruit Industries.
  Adafruit IO example additions by Todd Treece.
  MIT license, all text above must be included in any redistribution
 ****************************************************/
#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

// function prototypes
void connect(void);

/****************************** Pins ******************************************/

//none

/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "Startup_Edmonton_GUEST"    // Your WiFi AP name.
#define WLAN_PASS       "Mercer2016"     // Your WiFi AP password.

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "pylondude" // Adafruit IO username (see http://accounts.adafruit.com).
#define AIO_KEY         "8f25cf8c251f4acd9234a589388679a0"      // Adafruit IO key

/************ Global State (you don't need to change this!) ******************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

// Setup a feed called 'button' for publishing changes.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
//const char BUTTON_FEED[] PROGMEM = AIO_USERNAME "/feeds/button"; //OLD DO NOT USE
//const char BUTTON_FEED[] = AIO_USERNAME "/feeds/button";
//Adafruit_MQTT_Publish button = Adafruit_MQTT_Publish(&mqtt, BUTTON_FEED);

/****************************** Feeds ***************************************/

// Setup a feed called 'theTime' for subscribing to changes.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
//const char TIME_FEED[] PROGMEM = AIO_USERNAME "/feeds/theTime"; //OLD CODE
//const char TIME_FEED[] = AIO_USERNAME "/feeds/time/seconds";
Adafruit_MQTT_Subscribe theTime = Adafruit_MQTT_Subscribe(&mqtt, "time/seconds");

/*************************** Sketch Code ************************************/

//doesn't account for daylight savings
int timeZone = -7;

void setup() {

  Serial.begin(115200);

  Serial.println(F("Adafruit IO Example"));

  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  delay(10);
  Serial.print(F("Connecting to "));
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
  }
  Serial.println();

  Serial.println(F("WiFi connected"));
  Serial.println(F("IP address: "));
  Serial.println(WiFi.localIP());

  // listen for events on the theTime feed
  mqtt.subscribe(&theTime);

  // connect to adafruit io
  connect();

}

void loop() {

  Adafruit_MQTT_Subscribe *subscription;

  // ping adafruit io a few theTimes to make sure we remain connected
  if(! mqtt.ping(3)) {
    // reconnect to adafruit io
    if(! mqtt.connected())
      connect();
  }

  // this is our 'wait for incoming subscription packets' busy subloop
  while (subscription = mqtt.readSubscription(1000)) {

    // we only care about the theTime events
    if (subscription == &theTime) {

      // convert mqtt ascii payload to int
      char *value = (char *)theTime.lastread;
      Serial.print(F("Received: "));
      Serial.println(value);

      // do stuff with value
      Serial.print(atoi(value)/60/60%24+timeZone); //find hours
      Serial.print(":");
      Serial.println(atoi(value)/60%60); // find minutes

    }

  }

}

// connect to adafruit io via MQTT
void connect() {

  Serial.print(F("Connecting to Adafruit IO... "));

  int8_t ret;

  while ((ret = mqtt.connect()) != 0) {

    switch (ret) {
      case 1: Serial.println(F("Wrong protocol")); break;
      case 2: Serial.println(F("ID rejected")); break;
      case 3: Serial.println(F("Server unavail")); break;
      case 4: Serial.println(F("Bad user/pass")); break;
      case 5: Serial.println(F("Not authed")); break;
      case 6: Serial.println(F("Failed to subscribe")); break;
      default: Serial.println(F("Connection failed")); break;
    }

    if(ret >= 0)
      mqtt.disconnect();

    Serial.println(F("Retrying connection..."));
    delay(5000);

  }

  Serial.println(F("Adafruit IO Connected!"));

}
