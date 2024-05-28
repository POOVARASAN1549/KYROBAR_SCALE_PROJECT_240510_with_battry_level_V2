#include "HX711.h"
HX711 scale;

uint8_t dataPin = 6;
uint8_t clockPin = 7;

uint8_t BUTTON_PIN = 2;
uint8_t MOSFET = 3;
uint8_t TAB_POWER = 4;
uint8_t TAB_OTG = 5;
bool flag = LOW;

#define SHORT_PRESS_TIME 500  // 500 milliseconds
#define LONG_PRESS_TIME 2000  // 3000 milliseconds

// Variables will change:
int lastState = LOW;  // the previous state from the input pin
int currentState;     // the current reading from the input pin
unsigned long pressedTime = 0;
unsigned long releasedTime = 0;

int previousLoadCellData = 0;
const int stabilityThreshold = 5;                // Define your threshold here
const unsigned long stabilityDuration = 200000;  // Define stability duration in milliseconds
unsigned long lastStableTime = 0;

bool flag1 = false;

uint8_t batt = A7;
uint8_t buzz = 10;

void setup() {
  Serial.begin(9600);

  scale.begin(dataPin, clockPin);

  pinMode(BUTTON_PIN, INPUT);
  pinMode(batt, INPUT);
  pinMode(buzz, OUTPUT);


  pinMode(MOSFET, OUTPUT);
  digitalWrite(MOSFET, HIGH);  // turn the MOSFET on (HIGH is the voltage level)
  Serial.println("device on");
  delay(50);

  pinMode(TAB_POWER, OUTPUT);
  digitalWrite(TAB_POWER, HIGH);  // turn the TAB_OTG on (HIGH is the voltage level)
  delay(50);

  pinMode(TAB_OTG, OUTPUT);
  digitalWrite(TAB_OTG, HIGH);  // turn the TAB_OTG on (HIGH is the voltage level)
  delay(50);

  // load cell factor for 5 KG
  scale.set_scale(-401.79);  // TODO you need to calibrate this yourself

  // reset the scale to zero = 0
  scale.tare();

  digitalWrite(TAB_POWER, HIGH);  // turn the TAB_POWER on (HIGH is the voltage level)
  delay(2000);
  digitalWrite(TAB_POWER, LOW);  // turn the TAB_POWER on (HIGH is the voltage level)
}




void loop() {

   // read the input on analog pin 0:
  int sensorValue = analogRead(batt);
  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
  float voltage = sensorValue * (5.0 / 1023.0);
  // print out the value you read:
  Serial.println(voltage);

  if (voltage < 1.6)
  {
    digitalWrite(buzz, HIGH);
    delay(500);
    digitalWrite(buzz, LOW);
  }

  int currentLoadCellData = readLoadCellData();
  // Serial.println(currentLoadCellData);

  if (isStable(currentLoadCellData)) {

    if (millis() - lastStableTime >= stabilityDuration && flag == false) {
      Serial.println("sleep mode");
      flag = true;
      digitalWrite(TAB_OTG, LOW);  // turn the TAB_OTG on (HIGH is the voltage level)
      delay(10);
      digitalWrite(TAB_POWER, HIGH);  // turn the TAB_POWER on (HIGH is the voltage level)
      delay(1000);
      digitalWrite(TAB_POWER, LOW);  // turn the TAB_POWER on (HIGH is the voltage level)
      delay(10);

      digitalWrite(MOSFET, LOW);  // turn the MOSFET off (LOW is the voltage level)
      delay(500);
    }
  } else {
    lastStableTime = millis();
    previousLoadCellData = currentLoadCellData;
    flag = false;
  }

  // read the state of the switch/button:
  currentState = digitalRead(BUTTON_PIN);
  //  Serial.println(currentState);
  if (lastState == HIGH && currentState == LOW)  // button is pressed
    pressedTime = millis();
  else if (lastState == LOW && currentState == HIGH) {  // button is released
    releasedTime = millis();

    long pressDuration = releasedTime - pressedTime;

    if (pressDuration < LONG_PRESS_TIME) {
      Serial.println("A short press is detected");
      scale.tare();
    }
    if (pressDuration > LONG_PRESS_TIME) {
      Serial.println("A long press is detected");
      Serial.println("device off");
      digitalWrite(TAB_OTG, LOW);  // turn the TAB_OTG on (HIGH is the voltage level)
      delay(10);
      digitalWrite(TAB_POWER, HIGH);  // turn the TAB_POWER on (HIGH is the voltage level)
      delay(1000);
      digitalWrite(TAB_POWER, LOW);  // turn the TAB_POWER on (HIGH is the voltage level)
      delay(10);
      digitalWrite(MOSFET, LOW);  // turn the MOSFET off (LOW is the voltage level)
      delay(500);
    }
  }

  // save the the last state
  lastState = currentState;

  if (flag == false) Serial.println(readLoadCellData());
}

int readLoadCellData() {
  // Read load cell data
  return scale.get_units(5);
}

bool isStable(int currentLoadCellData) {

  return abs(currentLoadCellData - previousLoadCellData) <= stabilityThreshold;
}


// -- END OF FILE --
