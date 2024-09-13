#include <Servo.h>

const byte outsideSensorPin = D3 , insideSensorPin = D0 , firstSensorPin = D1,  secondSensorPin = D2 , servoPin = D5, voltagePin = D4;


Servo garageDoorServo;

bool isDoorOpen = false;
const unsigned long closeTimer = 3000;
unsigned long prevTime = 0;

void openGarage(){
    isDoorOpen = true;
    prevTime = millis();
    garageDoorServo.write(90);
}

void closeGarage(){
    isDoorOpen = false;
    garageDoorServo.write(180);
}



void garageSetup(){
    pinMode(outsideSensorPin, INPUT);
    pinMode(insideSensorPin, INPUT);
    pinMode(firstSensorPin, INPUT);
    pinMode(secondSensorPin, INPUT);

    garageDoorServo.attach(servoPin);

    garageDoorServo.write(0);
}

void setup() {
  pinMode(voltagePin, OUTPUT);
  digitalWrite(voltagePin, HIGH);
  Serial.begin(115200);
    garageSetup();
}


byte garageCount() {
  bool firstSensor = !digitalRead(firstSensorPin);
  bool secondSensor = !digitalRead(secondSensorPin);
  return firstSensor + secondSensor;
}

void garageHandler(){
  // open
    bool outsideSensor = !digitalRead(outsideSensorPin);
    bool insideSensor = !digitalRead(insideSensorPin);
    byte currentCarCnt = garageCount();

      if (insideSensor)
        openGarage();
      else if(currentCarCnt < 2 && outsideSensor)
        openGarage();
        // if both sensors don't detect anything (could've opened then left) or numbers of car in garage changed (car entered or left)

    bool closeCheck = isDoorOpen && ((outsideSensor && currentCarCnt < 2) + insideSensor == 0);
    closeCheck = closeCheck && (millis() - prevTime) >= closeTimer;

    if(closeCheck)
      closeGarage();
}

void loop() {
    garageHandler();
}