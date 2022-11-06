#include <AccelStepper.h>
#include <Wire.h>

void dataRcv(int numBytes);


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

typedef struct {
  bool up = 0;
  bool down = 0;
  bool left = 0;
  bool right = 0;
  bool speed_led_toggle = 0;
  bool speed_led = 0; // start in high speed mode (to indicate initial power bc of LED)
  int16_t x = 0; // value from 0-4 indicating speed
  int16_t y = 0; // value from 0-4 indicating speed
} control_t;

volatile static control_t controls;

void setup() {
  Wire.begin(0x08);           // join I2C bus as Slave with address 0x08
  // event handler initializations
  Wire.onReceive(dataRcv);    // register an event handler for received data

  // enable stepper motor drivers
  pinMode(8, OUTPUT);
  digitalWrite(8, LOW);

  //  pinMode(altDirPin, OUTPUT);
  //  digitalWrite(altDirPin, HIGH);
  //
  //  pinMode(altStepPin, OUTPUT);
  //  digitalWrite(altStepPin, HIGH);
  Serial.begin(9600);
  //Serial2.begin(9600, SERIAL_7O1);

  // set the maximum speed, acceleration factor,
  // initial speed and the target position
  AltStepper.setMaxSpeed(MAX_SPEED);
  AltStepper.setAcceleration(MAX_ACCEL);

  AzStepper.setMaxSpeed(MAX_SPEED);
  AzStepper.setAcceleration(MAX_ACCEL);
}

void loop() {
  //Serial.print("here2");

  Serial.print("Up: ");
  Serial.print(controls.up);
  Serial.print(", Down: ");
  Serial.print(controls.down);
  Serial.print(", Left: ");
  Serial.print(controls.left);
  Serial.print(", Right: ");
  Serial.print(controls.right);
  Serial.print(", Y: ");
  Serial.print(controls.y);
  Serial.print(", X: ");
  Serial.print(controls.x);
  Serial.println();
  //  // Change direction once the motor reaches target position
  //  if (myStepper.distanceToGo() == 0)
  //    myStepper.moveTo(-myStepper.currentPosition());
  //
  //  // Move the motor one step
  //  myStepper.run();
}

//received data handler function
void dataRcv(int numBytes) {
  static uint8_t tempX;
  while (Wire.available()) {
    for (int i = 0; i < numBytes; i++) { // should read 4 bytes
      byte inData = Wire.read(); // throw out first value
      switch (i) {
        case 0:
          controls.left = inData & 1;
          controls.right = inData & (1 << 1);
          controls.up = inData & (1 << 2);
          controls.down = inData & (1 << 3);
          controls.speed_led = inData & (1 << 4);
          break;
        case 1:
          controls.x = inData;
          break;
        case 2:
          controls.y = inData;
          break;
      }
    }
  }
}
