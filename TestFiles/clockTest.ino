#include <ShiftRegister74HC595.h>

/*
//Pin connected to SH_CP of 74HC595
#define CLOCK_PIN   12
////Pin connected to DS of 74HC595
#define DATA_PIN    13
//Pin connected to ST_CP of 74HC595
#define LATCH_PIN   14
*/

int numberOfShiftRegisters = 2; // number of shift registers attached in series
int serialDataPin = 13; // DS
int clockPin = 12; // SHCP
int latchPin = 14; // STCP
ShiftRegister74HC595 sr (numberOfShiftRegisters, serialDataPin, clockPin, latchPin);

//digit shift register codes
uint8_t digit_0  = B01011111;
uint8_t digit_1  = B01000001;
uint8_t digit_2  = B00111011;
uint8_t digit_3  = B01110011;
uint8_t digit_4  = B01100101;
uint8_t digit_5  = B01110110;
uint8_t digit_6  = B01111110;
uint8_t digit_7  = B01000011;
uint8_t digit_8  = B01111111;
uint8_t digit_9  = B01110111;
uint8_t no_digit = B00000000;

//selector and dot register codes
uint8_t first_digit  = B00011110;
uint8_t second_digit = B00101110;
uint8_t third_digit  = B00110110;
uint8_t fourth_digit = B00111010;
//need to do addition to include information dots like am/pm
uint8_t center_colon = B01111100;

void setup() {
  //Start Serial for debuging purposes  
  Serial.begin(115200);
  //set pins to output because they are addressed in the main loop
  //pinMode(CLOCK_PIN, OUTPUT);
  //pinMode(DATA_PIN, OUTPUT);
  //pinMode(LATCH_PIN, OUTPUT);

  // set multiple pins at once when using two shift registers in series
  uint8_t pinValues[] = { no_digit, center_colon };
  sr.setAll(pinValues);
  
  Serial.println("Output Complete");
}

void loop() {
  delay(100);
  /*digitalWrite(DATA_PIN, HIGH);
  for(int i = 1; i <= 8; i++){
    digitalWrite(CLOCK_PIN, HIGH);
    delay(10);
    digitalWrite(CLOCK_PIN, LOW);
    delay(10);
  }

  digitalWrite(DATA_PIN, LOW);
  for(int i = 1; i <= 5; i++){
    digitalWrite(CLOCK_PIN, HIGH);
    delay(10);
    digitalWrite(CLOCK_PIN, LOW);
    delay(10);
  }

  digitalWrite(DATA_PIN, HIGH);
  for(int i = 1; i <= 3; i++){
    digitalWrite(CLOCK_PIN, HIGH);
    delay(10);
    digitalWrite(CLOCK_PIN, LOW);
    delay(10);
  }

  digitalWrite(LATCH_PIN, HIGH);
  delay(10);
  digitalWrite(LATCH_PIN, LOW);
  Serial.println("outputing");
  delay(1000);*/
}
