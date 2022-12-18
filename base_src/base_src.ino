//#include <AccelStepper.h>
#include <Wire.h>
#include <stdlib.h>

void dataRcv(int numBytes);

#define DEBUG

// Define pin connections
#define X_DIR_PIN 5
#define X_STEP_PIN 2
#define Y_DIR_PIN 6
#define Y_STEP_PIN 3

#define STEPPER_EN 8

// joystick returns value 0-255
#define STICK_MIN 0
#define STICK_MAX 255
#define STICK_MID 128
#define STICK_DEADZONE 15 // 25 from center is dead zone

#define STICK_MOVE_THRESH1 STICK_DEADZONE
#define STICK_MOVE_THRESH2 -STICK_DEADZONE

#define MIN_SPEED_DELAY 5000
#define MAX_SPEED_DELAY 30000

// control inputs
typedef struct {
  // buttons
  bool up = 0;
  bool down = 0;
  bool left = 0;
  bool right = 0;
  bool en_led = false; // start with steppers disabled
  // joy stick directions
  int16_t x = 0;
  int16_t y = 0;
} control_t;

volatile static control_t controls;

int16_t xStepDelay = 0;
int16_t yStepDelay = 0;


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

  cli();
  
  //set timer0 interrupt at 2kHz
  TCCR0A = 0;// set entire TCCR0A register to 0
  TCCR0B = 0;// same for TCCR0B
  TCNT0  = 0;//initialize counter value to 0
  // set compare match register for 2khz increments
  OCR0A = 6;// = (16*10^6) / (2000*64) - 1 (must be <256)
  // turn on CTC mode
  TCCR0A |= (1 << WGM01);
  // Set CS01 and CS00 bits for 64 prescaler
  TCCR0B |= (1 << CS01) | (1 << CS00);   
  // enable timer compare interrupt
  TIMSK0 |= (1 << OCIE0A);
  
  sei();
  
}

  static int16_t delayX = 0;
  static int16_t delayY = 0;
  static bool onX = false;
  static bool onY = false;
  static bool up_pressed = false;
  static bool down_pressed = false;
  static bool left_pressed = false;
  static bool right_pressed = false;

ISR(TIMER0_COMPA_vect){ // occurs every approx. 100us

  // X direction
  if(delayX == 0){
    digitalWrite(X_STEP_PIN, LOW);
      // round to nearest 100us
      delayX = (abs(controls.x) > 25) ? (xStepDelay / 100) : 0; // assign only if out of deadzone
      if(left_pressed){
        delayX++;
        left_pressed = false;
      } else if(right_pressed) {
        delayX++;
        right_pressed = false;
      }
      onX = false;
  } else {
    if(!onX){
      digitalWrite(X_STEP_PIN, HIGH);
      onX = true;
    } else {
      delayX--;
    }
  }

  // Y direction
  if(delayY == 0){
    digitalWrite(Y_STEP_PIN, LOW);
      // round to nearest 100us
      delayY = (abs(controls.y) > 25) ? (yStepDelay / 100) : 0; // assign only if out of deadzone
      if(up_pressed){
        delayY++;
        up_pressed = false;
      } else if(down_pressed) {
        delayY++;
        down_pressed = false;
      }
      onY = false;
  } else {
    if(!onY){
      digitalWrite(Y_STEP_PIN, HIGH);
      onY = true;
    } else {
      delayY--;
    }
  }
}

void loop() {
  
  xStepDelay = map(abs(controls.x), STICK_MIN, STICK_MID, MAX_SPEED_DELAY, MIN_SPEED_DELAY);
  yStepDelay = map(abs(controls.y), STICK_MIN, STICK_MID, MAX_SPEED_DELAY, MIN_SPEED_DELAY);
  /*
   * Motor enable
   */

   static bool en = false; // used so steppers aren't continually being enabled & disabled
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
  static bool currXDir = LOW;
  static bool currYDir = LOW;
  

  // set new X dir
  if(controls.x > STICK_DEADZONE && currXDir == HIGH){
    // only update pin state if needed
    digitalWrite(X_DIR_PIN, HIGH);
    currXDir = LOW;
  } else if(controls.x < -STICK_DEADZONE && currXDir == LOW){
    digitalWrite(X_DIR_PIN, LOW);
    currXDir = HIGH;
  }

  // set new Y dir
  if(controls.y > STICK_DEADZONE && currYDir == HIGH){
    // only update pin state if needed
        digitalWrite(Y_DIR_PIN, LOW);
    currYDir = LOW;
  } else if(controls.y < -STICK_DEADZONE && currYDir == LOW){
      digitalWrite(Y_DIR_PIN, HIGH);
      currYDir = HIGH;
  }

  // Up button movements
  if(controls.up){
    while(controls.up); // wait for it to be released
    up_pressed = true;
    digitalWrite(Y_DIR_PIN, HIGH);
    currYDir = HIGH;
    }

  // Down button movements
  if(controls.down){
    while(controls.down); // wait for it to be released
    down_pressed = true;
    digitalWrite(Y_DIR_PIN, LOW);
    currYDir = LOW;
  }
  
  // Left button movements
  if(controls.left){
    while(controls.left); // wait for it to be released
    left_pressed = true;
    digitalWrite(X_DIR_PIN, HIGH);
    currXDir = HIGH;
  }

  // Right button movements
  if(controls.right){
    while(controls.right); // wait for it to be released
    right_pressed = true;
    digitalWrite(X_DIR_PIN, LOW);
    currXDir = LOW;
  }

#ifdef DEBUG
//  Serial.print(", Y Speed Delay: ");
//  Serial.print(yStepDelay);
//  Serial.print(", X Speed Delay: ");
//  Serial.print(xStepDelay);
//  Serial.print(", X Dir: ");
//  Serial.print(currXDir);
//  Serial.print(", Y Dir: ");
//  Serial.print(currYDir);
//  Serial.print(", Y: ");
//  Serial.print(controls.y);
  Serial.print(", controls.up: ");
  Serial.print(controls.up);
  Serial.print(", controls.down: ");
  Serial.print(controls.down);
  Serial.print(", controls.left: ");
  Serial.print(controls.left);
  Serial.print(", controls.right: ");
  Serial.print(controls.right);
//  Serial.print(", delayX: ");
//  Serial.print(delayX);
//  Serial.print(", X: ");
//  Serial.print(controls.x);
  Serial.println();
#endif
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
          controls.y = inData - STICK_MID;
          break;
        case 2:
          controls.x = inData - STICK_MID;
          break;
      }
    }
  }
}
