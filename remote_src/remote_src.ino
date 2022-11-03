#define UP                PIN_PB3
#define DOWN              PIN_PB4
#define LEFT              PIN_PB6
#define RIGHT             PIN_PB1
#define X                 PIN_PD5
#define Y                 PIN_PD3
#define SPEED_LED         PIN_PA1
#define SPEED_LED_TOGGLE  PIN_PB0


#define MAX_ANALOG_VAL 

void setup() {
  // input buttons
  pinMode(UP, INPUT_PULLUP);
  pinMode(DOWN, INPUT_PULLUP);
  pinMode(LEFT, INPUT_PULLUP);
  pinMode(RIGHT, INPUT_PULLUP);
  pinMode(SPEED_LED_TOGGLE, INPUT_PULLUP);
  // variable voltage joystick inputs
  pinMode(X, INPUT);
  pinMode(Y, INPUT);
  // joystick speed indicator
  pinMode(SPEED_LED, OUTPUT);

  // Setup UART
  Serial.begin(9600, SERIAL_8O1); // enable odd parity
}


typedef struct {
  bool up = 0;
  bool down = 0;
  bool left = 0;
  bool right = 0;
  bool speed_led_toggle = 0;
  bool speed_led = 1; // start in high speed mode (to indicate initial power bc of LED)
  int x = 0; // value from 0-4 indicating speed
  int y = 0; // value from 0-4 indicating speed
} control_t

static control_t controls;

void loop() {
  // read buttons
  controls.up  = digitalRead(UP);
  controls.down = digitalRead(DOWN);
  controls.left = digitalRead(LEFT);
  controls.right = digitalRead(RIGHT);
  controls.speed_led_toggle = digitalRead(SPEED_LED_TOGGLE);
  
  // set speed LED
  if(controls.speed_led_toggle) speed_led = !speed_led;
  
  // read joystick X
  int tempX = analogRead(X);
  
  // read joystick Y
  int tempY = analogRead(Y);
  
  // combine into hex

  // send hex
  Serial.write(0x);
}
