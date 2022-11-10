#include <AccelStepper.h>
#include <Wire.h>

void dataRcv(int numBytes);

//#define DEBUG


// Define pin connections
#define xDirPin 5
#define xStepPin 2
#define yDirPin 6
#define yStepPin 3

#define STEPPER_EN 8

#define MAX_SPEED 200
#define SLOW_SPEED MAX_SPEED/3
#define ACCEL 100


// joystick returns value 0-255
// 255 <- X -> 0
// ^ 255 Y v 0
#define STICK_MIN 0
#define STICK_MAX 255
#define STICK_MID 128
#define STICK_DEADZONE 25 // 25 from center is dead zone
#define STICK_FASTZONE 50 // 50 from end is fast zone, the rest in the middle is slow zone

// Define motor interface type
#define motorInterfaceType 1

// Creates an instance
AccelStepper xStepper(motorInterfaceType, xStepPin, xDirPin);
AccelStepper yStepper(motorInterfaceType, yStepPin, yDirPin);

typedef struct {
  bool up = 0;
  bool down = 0;
  bool left = 0;
  bool right = 0;
  bool en_led_toggle = 0;
  bool en_led = 0; // start with steppers disabled
  int16_t x = 0;
  int16_t y = 0;
} control_t;

volatile static control_t controls;

typedef struct {
  int16_t x = 0;
  int16_t y = 0;
  int16_t xPos = 0;
  int16_t yPos = 0;
  int16_t xJoy = 0;
  int16_t yJoy = 0;
} move_t;

volatile static move_t toMove; // holds movements that need to be performed

void setup() {
  // Debug output
#ifdef DEBUG
  Serial.begin(9600);
#endif

  Wire.begin(0x08); // join I2C bus as Slave with address 0x08
  Wire.onReceive(dataRcv); // register an event handler for received data

  // start with stepper motors disabled
  pinMode(STEPPER_EN, OUTPUT);
  digitalWrite(STEPPER_EN, LOW);


  // set the maximum speed, acceleration factor,
  xStepper.setMaxSpeed(MAX_SPEED);

  yStepper.setMaxSpeed(MAX_SPEED);
  
  xStepper.setSpeed(SLOW_SPEED);
}

void loop() {
 
    // update needed movements
    switch(controls.x){
      case 1:
        xStepper.setSpeed(SLOW_SPEED);
        xStepper.move(1000000);
        break;
      case 2:
        xStepper.setSpeed(MAX_SPEED);
        xStepper.move(1000000);
        break;
      case -1:
        xStepper.setSpeed(-SLOW_SPEED);
        xStepper.move(1000000);
        break;
      case -2:
        xStepper.setSpeed(-MAX_SPEED);
        xStepper.move(1000000);
        break;
      case 0:
        xStepper.move(xStepper.currentPosition());
        break;
    }

    if(controls.left){
      while(controls.left); // wait for it to be released
      xStepper.setSpeed(SLOW_SPEED);
      xStepper.move(xStepper.currentPosition()+1);
    }
    if(controls.right){
      while(controls.right); // wait for it to be released
      xStepper.setSpeed(-SLOW_SPEED);
      xStepper.move(xStepper.currentPosition()+1);
    }
    
  //
  //  // make moves
  //  if(xStepper.distanceToGo() == 0 && toMove.x != toMove.xPos){ // if we're not moving
  //      xStepper.move(toMove.x);
  //  }
  xStepper.run();
  //  if(controls.left) toMove.alt--;
  //  if(controls.right) toMove.alt++;
  //  if(controls.up) toMove.az++;
  //  if(controls.down) toMove.az--;
#ifdef DEBUG
  Serial.print("toMove.x: ");
  Serial.print(toMove.x);
  Serial.print(", toMove.y: ");
  Serial.print(toMove.y);
  Serial.print(", En: ");
  Serial.print(controls.en_led);
  Serial.print(", Y: ");
  Serial.print(controls.y);
  Serial.print(", X: ");
  Serial.print(controls.x);
  Serial.println();
#endif

  //  // Change direction once the motor reaches target position
  //    if (AltStepper.distanceToGo() == 0)
  //      AltStepper.moveTo(-AltStepper.currentPosition());
  //    if (AzStepper.distanceToGo() == 0)
  //      AzStepper.moveTo(-AzStepper.currentPosition());
  // Move the motor one step
  //AltStepper.run();
  //AzStepper.run();
}

void AccelStepper::setOutputPins(uint8_t mask)
{
    uint8_t numpins = 2;
    for (uint8_t i = 0; i < numpins; i++)
  digitalWrite(_pin[i], (mask & (1 << i)) ? (HIGH ^ _pinInverted[i]) : (LOW ^ _pinInverted[i]));
}


void AccelStepper::step1(long step)
{
    (void)(step); // Unused

    // _pin[0] is step, _pin[1] is direction
    setOutputPins(_direction ? 0b10 : 0b00); // Set direction first else get rogue pulses
    setOutputPins(_direction ? 0b11 : 0b01); // step HIGH
    // Caution 200ns setup time 
    // Delay the minimum allowed pulse width
    delayMicroseconds(1);
    setOutputPins(_direction ? 0b10 : 0b00); // step LOW
}

//received data handler function
void dataRcv(int numBytes) {
  while (Wire.available()) {
    for (int i = 0; i < numBytes; i++) { // should read 3 bytes
      byte inData = Wire.read(); // throw out first value
      switch (i) {
        case 0:
          controls.left = !(inData & 1);
          controls.right = !(inData & (1 << 1));
          controls.up = !(inData & (1 << 2));
          controls.down = !(inData & (1 << 3));
          controls.en_led = inData & (1 << 4);
          break;
        case 1:
          // Y axis is backwards
          if (inData > STICK_MID - STICK_DEADZONE && inData < STICK_MID + STICK_DEADZONE) // in deadzone
            controls.y = 0;
          else if (inData < STICK_MIN + STICK_FASTZONE) // positive fast
            controls.y = 2;
          else if (inData > STICK_MAX - STICK_FASTZONE) // negative fast
            controls.y = -2;
          else if (inData < STICK_MID - STICK_DEADZONE) // negative slow
            controls.y = 1;
          else if (inData > STICK_MID + STICK_DEADZONE)
            controls.y = -1;
          break;
        case 2:
          // X axis is backwards
          if (inData > STICK_MID - STICK_DEADZONE && inData < STICK_MID + STICK_DEADZONE) // in deadzone
            controls.x = 0;
          else if (inData < STICK_MIN + STICK_FASTZONE) // positive fast
            controls.x = 2;
          else if (inData > STICK_MAX - STICK_FASTZONE) // negative fast
            controls.x = -2;
          else if (inData < STICK_MID - STICK_DEADZONE) // positive slow
            controls.x = 1;
          else if (inData > STICK_MID + STICK_DEADZONE)
            controls.x = -1;
          break;
      }
    }
  }
}
