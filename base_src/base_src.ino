#include <AccelStepper.h>

// Define pin connections
const int altDirPin = 5;
const int altStepPin = 2;

const int AzDirPin = 6;
const int AzStepPin = 3;

#define MAX_SPEED 1000
#define MAX_ACCEL 30

// Define motor interface type
#define motorInterfaceType 1

// Creates an instance
AccelStepper AltStepper(motorInterfaceType, altStepPin, altDirPin);
AccelStepper AzStepper(motorInterfaceType, AzStepPin, AzDirPin);

void setup() {

  // enable stepper motor drivers
  pinMode(8, OUTPUT);
  digitalWrite(8, LOW);

//  pinMode(altDirPin, OUTPUT);
//  digitalWrite(altDirPin, HIGH);
//
//  pinMode(altStepPin, OUTPUT);
//  digitalWrite(altStepPin, HIGH);

  // set the maximum speed, acceleration factor,
  // initial speed and the target position
  AltStepper.setMaxSpeed(MAX_SPEED);
  AltStepper.setAcceleration(MAX_ACCEL);

  AzStepper.setMaxSpeed(MAX_SPEED);
  AzStepper.setAcceleration(MAX_ACCEL);
}

void loop() {
  // Change direction once the motor reaches target position
  if (myStepper.distanceToGo() == 0) 
    myStepper.moveTo(-myStepper.currentPosition());

  // Move the motor one step
  myStepper.run();
}
