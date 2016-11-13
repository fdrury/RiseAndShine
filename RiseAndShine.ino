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
#include <ShiftRegister74HC595.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

// function prototypes
void connect(void);
void clockUpdate(int theHour, int theMinute);
void alarm(void);
void sendSwitchValue(int32_t value);

/****************************** Pins *****************************************/

#define BUZZER_0        15
#define BUZZER_1        16
#define LIGHT_PIN       4
#define SWITCH_PIN      5

/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "Startup_Edmonton_GUEST"    // Your WiFi AP name.
#define WLAN_PASS       "Mercer2016"     // Your WiFi AP password.

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "pylondude" // Adafruit IO username (see http://accounts.adafruit.com).
#define AIO_KEY         "8f25cf8c251f4acd9234a589388679a0"      // Adafruit IO key

/***************************** Clock Setup ***********************************/

int numberOfShiftRegisters = 2; // number of shift registers attached in series
int serialDataPin = 13; // DS
int clockPin = 12; // SHCP
int latchPin = 14; // STCP
ShiftRegister74HC595 sr (numberOfShiftRegisters, serialDataPin, clockPin, latchPin);

//digit shift register codes
uint8_t digitArray[10];
uint8_t no_digit = B00000000;

//selector and dot register codes
uint8_t first_digit  = B00011110;
uint8_t second_digit = B00101110;
uint8_t third_digit  = B00110110;
uint8_t fourth_digit = B00111010;
//need to do addition to include information dots like am/pm
uint8_t center_colon = B01111100;

int currentHour = 0;
int currentMinute = 0;

/********* Global State (Updated to reflect changes to Adafruit IO) **********/

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
const char ALARM_FEED[] = AIO_USERNAME "/feeds/alarm-calendar";
Adafruit_MQTT_Subscribe alarmer = Adafruit_MQTT_Subscribe(&mqtt, ALARM_FEED);
const char SWITCH_FEED[] = AIO_USERNAME "/feeds/light-switch";
Adafruit_MQTT_Subscribe lightSwitchSub = Adafruit_MQTT_Subscribe(&mqtt, SWITCH_FEED);
Adafruit_MQTT_Publish lightSwitchPub = Adafruit_MQTT_Publish(&mqtt, SWITCH_FEED);

/*************************** Sketch Code ************************************/

//doesn't account for daylight savings
int timeZone = -7;

bool lightsOn = false;
bool ignoreNext = false; //prevents Adafruit IO feedback loop
bool switchState;

void setup() {

  Serial.begin(115200);

  Serial.println(F("Begin Rise and Shine"));

  //Initialize digitArray
  digitArray[0] = B01011111;
  digitArray[1] = B01000001;
  digitArray[2] = B00111011;
  digitArray[3] = B01110011;
  digitArray[4] = B01100101;
  digitArray[5] = B01110110;
  digitArray[6] = B01111110;
  digitArray[7] = B01000011;
  digitArray[8] = B01111111;
  digitArray[9] = B01110111;

  //initializee output pins
  pinMode(BUZZER_0, OUTPUT);
  pinMode(BUZZER_1, OUTPUT);
  pinMode(LIGHT_PIN, OUTPUT);
  pinMode(SWITCH_PIN, INPUT);

  switchState = digitalRead(SWITCH_PIN);

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
  mqtt.subscribe(&alarmer);
  mqtt.subscribe(&lightSwitchSub);

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

      currentHour = atoi(value)/60/60%24+timeZone;
      currentMinute = atoi(value)/60%60;
      
      // do stuff with value
      Serial.print(currentHour);
      Serial.print(":");
      if(currentMinute/10 == 0)
        Serial.print("0");
      Serial.println(currentMinute); //looks wrong with leading 0 (omitted) -> okay because only used for debugging.
    }

    if (subscription == &alarmer){
      char *value = (char *)alarmer.lastread;
      Serial.print(F("Received: "));
      Serial.println(value);

      alarm();
    }

    if (subscription == &lightSwitchSub){
      char *value = (char *)lightSwitchSub.lastread;
      Serial.print(F("Received: "));
      Serial.println(value);

      if(!ignoreNext){      
        //turn on light (REMEMBER server part of this)
        lightsOn = atoi(value);
        digitalWrite(LIGHT_PIN, lightsOn);
        sendSwitchValue(lightsOn);
      }
      else{
        ignoreNext = false; //reset state
        Serial.println("ignored");
      }
    }
    
    Serial.print("STATE------");
    Serial.println(switchState);
    if(switchState != digitalRead(SWITCH_PIN)){
      //toggle light (REMEMBER server part of this)
      switchState = digitalRead(SWITCH_PIN);
      lightsOn++;
      digitalWrite(LIGHT_PIN, lightsOn);
      sendSwitchValue(lightsOn);
    }

    clockUpdate(currentHour, currentMinute);
  }

  Serial.print("STATE------");
  Serial.println(switchState);
  if(switchState != digitalRead(SWITCH_PIN)){
    //toggle light (REMEMBER server part of this)
    switchState = digitalRead(SWITCH_PIN);
    lightsOn++;
    digitalWrite(LIGHT_PIN, lightsOn);
    sendSwitchValue(lightsOn);
  }
  
  clockUpdate(currentHour, currentMinute);
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

// update 7-segment clock
void clockUpdate(int theHour, int theMinute){
  int clockDelay = 3;
  
  uint8_t pinValues[] = { digitArray[theHour/10], first_digit };
  int startTime = millis();
  while(millis()-startTime < 1000){ // hold on
    if(theHour/10 != 0){ // otherwise won't be shown
      sr.setAll(pinValues);
      delay(clockDelay);
    }
    pinValues[0] = digitArray[theHour%10];
    pinValues[1] = second_digit;
    sr.setAll(pinValues);
    delay(clockDelay);
    pinValues[0] = no_digit;
    pinValues[1] = center_colon;
    sr.setAll(pinValues);
    delay(clockDelay);
    pinValues[0] = digitArray[theMinute/10];
    pinValues[1] = third_digit;
    sr.setAll(pinValues);
    delay(clockDelay);
    pinValues[0] = digitArray[theMinute%10];
    pinValues[1] = fourth_digit;
    sr.setAll(pinValues);
    delay(clockDelay);
    sr.setAllLow();//so that last digit isn't brighter
  }

  Serial.println("Clock Updated");
}

void alarm(){
  int delayTime = 100;
  int startTime = millis();

  //turn on light (REMEMBER server part of this)
  lightsOn = true;
  digitalWrite(LIGHT_PIN, lightsOn);
  sendSwitchValue(lightsOn);

  //ADD conditional (switch flicked)
  while(millis() - startTime < 100000){ //crashes if too long
    for(int i = 0; i < 100; i++){
      delayTime += 1;
      if(delayTime > 300)
        delayTime = 100;
      digitalWrite(BUZZER_0, HIGH);
      digitalWrite(BUZZER_1, LOW);
      delayMicroseconds(delayTime);
      digitalWrite(BUZZER_0, LOW);
      digitalWrite(BUZZER_1, HIGH);
      delayMicroseconds(delayTime);
    }
    delay(1); //required to give CPU time for other tasks
  }
}

void sendSwitchValue(int32_t value){
  Serial.print(F("\nSending button value: "));
  Serial.print(value);
  Serial.print("... ");

  if (! lightSwitchPub.publish(value))
    Serial.println(F("Failed."));
  else{
    Serial.println(F("Success!"));
    ignoreNext = true;
  }
}
