//#include <AccelStepper.h>
#include <Wire.h>

void dataRcv(int numBytes);

//#define DEBUG


// Define pin connections
#define X_DIR_PIN 5
#define X_STEP_PIN 2
#define Y_DIR_PIN 6
#define Y_STEP_PIN 3

#define STEPPER_EN 8

// joystick returns value 0-255
// 255 <- X -> 0
// ^ 255 Y v 0
#define STICK_MIN 0
#define STICK_MAX 255
#define STICK_MID 128
#define STICK_DEADZONE 25 // 25 from center is dead zone
#define STICK_FASTZONE 50 // 50 from end is fast zone, the rest in the middle is slow zone

#define MIN_SPEED_DELAY 3000
#define MAX_SPEED_DELAY 10000
typedef struct {
  bool up = 0;
  bool down = 0;
  bool cw = 0;
  bool ccw = 0;
  bool en_led_toggle = 0;
  bool en_led = 0; // start with steppers disabled
  int16_t x = 0;
  int16_t y = 0;
} control_t;

volatile static control_t controls;

void setup() {
  // Debug output
  #ifdef DEBUG
    Serial.begin(9600);
  #endif

  Wire.begin(0x08); // join I2C bus as Slave with address 0x08
  Wire.onReceive(dataRcv); // register an event handler for received data

  // start with stepper motors disabled
  pinMode(STEPPER_EN, OUTPUT);
  digitalWrite(STEPPER_EN, HIGH);

  // Setup X pins
  pinMode(X_DIR_PIN, OUTPUT);
  pinMode(X_STEP_PIN, OUTPUT);

  digitalWrite(X_STEP_PIN, LOW);
  digitalWrite(X_DIR_PIN, LOW);

  // Setup Y pins
  pinMode(Y_DIR_PIN, OUTPUT);
  pinMode(Y_STEP_PIN, OUTPUT);
  
  digitalWrite(Y_STEP_PIN, LOW);
  digitalWrite(Y_DIR_PIN, LOW);

  
}

void loop() {
  /*
   * Motor enable
   */

   static bool en = false; // used to steppers aren't continually being enabled & disabled
    if(controls.en_led && en == false){
      digitalWrite(STEPPER_EN, LOW); // enable steppers
      en = true;
    } else if(!controls.en_led && en == true){
      digitalWrite(STEPPER_EN, HIGH); // disable steppers
      en = false;
    }

   /*
    *  X Movements
    */

  // Joystick X-axis movements
  int xStepDelay = 3000;
  if (!(controls.x > STICK_MID - STICK_DEADZONE && controls.x < STICK_MID + STICK_DEADZONE)){ // if not in deadzone
    if(controls.x < STICK_MID - STICK_DEADZONE){ // positive
      xStepDelay = map(controls.x, STICK_MID - STICK_DEADZONE, STICK_MIN, MAX_SPEED_DELAY, MIN_SPEED_DELAY); // Convrests the read values of the potentiometer from 0 to 255 into desireded delay values (300 to 4000)  // Makes pules with custom delay, depending on the Potentiometer, from which the speed of the motor depends
      digitalWrite(X_DIR_PIN, LOW);
    } else { // negative
      xStepDelay = map(controls.x, STICK_MID + STICK_DEADZONE, STICK_MAX, MAX_SPEED_DELAY, MIN_SPEED_DELAY); // Convrests the read values of the potentiometer from 0 to 255 into desireded delay values (300 to 4000)  // Makes pules with custom delay, depending on the Potentiometer, from which the speed of the motor depends
      digitalWrite(X_DIR_PIN, HIGH);
    }
    // take 1 step
    //Serial.println(xStepDelay);
    digitalWrite(X_STEP_PIN, HIGH);
    delayMicroseconds(xStepDelay);
    digitalWrite(X_STEP_PIN, LOW);
    delayMicroseconds(xStepDelay);
  }

  // Left button movements
  if(controls.cw){
    while(controls.cw); // wait for it to be released
    digitalWrite(X_DIR_PIN, HIGH);
    digitalWrite(X_STEP_PIN, HIGH);
    delayMicroseconds(MAX_SPEED_DELAY);
    digitalWrite(X_STEP_PIN, LOW);
    delayMicroseconds(MAX_SPEED_DELAY);
  }

  // Right button movements
  if(controls.ccw){
    while(controls.ccw); // wait for it to be released
    digitalWrite(X_DIR_PIN, LOW);
    digitalWrite(X_STEP_PIN, HIGH);
    delayMicroseconds(MAX_SPEED_DELAY);
    digitalWrite(X_STEP_PIN, LOW);
    delayMicroseconds(MAX_SPEED_DELAY);
  }

  /*
   * Y Movements
   */

  // Joystick Y-axis movements
  int yStepDelay = 3000;
  if (!(controls.y > STICK_MID - STICK_DEADZONE && controls.y < STICK_MID + STICK_DEADZONE)){ // if not in deadzone
    if(controls.y < STICK_MID - STICK_DEADZONE){ // positive
      yStepDelay = map(controls.y, STICK_MID - STICK_DEADZONE, STICK_MAX, MAX_SPEED_DELAY, MIN_SPEED_DELAY); // Convrests the read values of the potentiometer from 0 to 255 into desireded delay values (300 to 4000)  // Makes pules with custom delay, depending on the Potentiometer, from which the speed of the motor depends
      digitalWrite(Y_DIR_PIN, LOW);
    } else { // negative
      yStepDelay = map(controls.y, STICK_MID + STICK_DEADZONE, STICK_MIN, MAX_SPEED_DELAY, MIN_SPEED_DELAY); // Convrests the read values of the potentiometer from 0 to 255 into desireded delay values (300 to 4000)  // Makes pules with custom delay, depending on the Potentiometer, from which the speed of the motor depends
      digitalWrite(Y_DIR_PIN, HIGH);
    }
    // take 1 step
    digitalWrite(Y_STEP_PIN, HIGH);
    delayMicroseconds(yStepDelay);
    digitalWrite(Y_STEP_PIN, LOW);
    delayMicroseconds(yStepDelay);
  }

  // Up button movements
  if(controls.up){
    while(controls.up); // wait for it to be released
    digitalWrite(Y_DIR_PIN, LOW);
    digitalWrite(Y_STEP_PIN, HIGH);
    delayMicroseconds(MAX_SPEED_DELAY);
    digitalWrite(Y_STEP_PIN, LOW);
    delayMicroseconds(MAX_SPEED_DELAY);
  }

  // Down button movements
  if(controls.down){
    while(controls.down); // wait for it to be released
    digitalWrite(Y_DIR_PIN, HIGH);
    digitalWrite(Y_STEP_PIN, HIGH);
    delayMicroseconds(MAX_SPEED_DELAY);
    digitalWrite(Y_STEP_PIN, LOW);
    delayMicroseconds(MAX_SPEED_DELAY);
  }

#ifdef DEBUG
//  Serial.print("toMove.x: ");
//  Serial.print(toMove.x);
//  Serial.print(", toMove.y: ");
//  Serial.print(toMove.y);
//  Serial.print(", En: ");
//  Serial.print(controls.en_led);
//  Serial.print(", Y: ");
//  Serial.print(controls.y);
//  Serial.print(", X: ");
//  Serial.print(controls.x);
//  Serial.println();
#endif
}

//received data handler function
void dataRcv(int numBytes) {
  while (Wire.available()) {
    for (int i = 0; i < numBytes; i++) { // should read 3 bytes
      byte inData = Wire.read(); // throw out first value
      switch (i) {
        case 0:
          controls.cw = !(inData & 1);
          controls.ccw = !(inData & (1 << 1));
          controls.up = !(inData & (1 << 2));
          controls.down = !(inData & (1 << 3));
          controls.en_led = inData & (1 << 4);
          break;
        case 1:
          controls.y = inData;
          break;
        case 2:
          controls.x = inData;
          break;
      }
    }
  }
}
