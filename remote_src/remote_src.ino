#define UP                PIN_PD2
#define DOWN              PIN_PB1
#define LEFT              PIN_PB2
#define RIGHT             PIN_PD5
#define X                 PIN_PC3
#define Y                 PIN_PC2
#define EN_LED         PIN_PB0
#define EN_LED_TOGGLE  PIN_PD7

// I2C master
#include <Wire.h>

#define EN_LED_START_VAL 0


typedef struct {
  bool up = 0;
  bool down = 0;
  bool left = 0;
  bool right = 0;
  bool en_led_toggle = 0;
  bool en_led = EN_LED_START_VAL; // start in high speed mode (to indicate initial power bc of LED)
  bool yIsNeg = 0;
  bool xIsNeg = 0;
  byte x = 0; // value from 0-4 indicating speed
  byte y = 0; // value from 0-4 indicating speed
  byte to_send[4];
} control_t;

static control_t controls;

void setup() {
  // input buttons
  pinMode(UP, INPUT_PULLUP);
  pinMode(DOWN, INPUT_PULLUP);
  pinMode(LEFT, INPUT_PULLUP);
  pinMode(RIGHT, INPUT_PULLUP);
  pinMode(EN_LED_TOGGLE, INPUT_PULLUP);
  // variable voltage joystick inputs
  pinMode(X, INPUT);
  pinMode(Y, INPUT);
  // joystick speed indicator
  pinMode(EN_LED, OUTPUT);

  // Setup I2C
  Wire.begin();

  digitalWrite(EN_LED, EN_LED_START_VAL);
}

void loop() {
  // read buttons
  controls.up  = digitalRead(UP);
  controls.down = digitalRead(DOWN);
  controls.left = digitalRead(LEFT);
  controls.right = digitalRead(RIGHT);
  controls.en_led_toggle = digitalRead(EN_LED_TOGGLE);
  
  // set speed LED
  if(!controls.en_led_toggle){
    while(!digitalRead(EN_LED_TOGGLE)); // wait for toggle to be released
    controls.en_led = !controls.en_led;
    digitalWrite(EN_LED, controls.en_led);
  }
  
  // read joystick X (
  controls.x = analogRead(X) / 4;

  // read joystick Y
  controls.y = analogRead(Y) / 4;

  // combine binary
  controls.to_send[0] = (controls.en_led << 4) | (controls.down << 3) | (controls.up << 2) | (controls.right << 1) | (controls.left) ;
  controls.to_send[1] = controls.x;
  controls.to_send[2] = controls.y;

  // send binary
  Wire.beginTransmission(0x08);
  Wire.write(controls.to_send, 3);
  Wire.endTransmission();

}
