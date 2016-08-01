#include <CheapStepper.h>

// ADJUSTABLE SETTINGS:
#define DEBUG true // true turns on Serial printing

CheapStepper stepper (8,9,10,11); // pins connected to IN1-IN4 on ULN2003
const int irLedPin = 12; // infrared led pin for camera trigger
const int buttonPin = 2; // button press input pin
const int camLedPin = 13; // shows 'cam ready' mode
const int moveLedPin = 4; // shows 'move ready' mode

const int turnsPerCircle = 64; // this is how many shots we want to take per revolution
const int gearRatioX100 = 25; // gear ratio of motor:turntable = 1:4 = 0.25 (* 100 = 25)
const int rpmMotor = 12; // 12rpm of motor at 1:4 gear ratio = 3rpm for turntable (~0.3 sec for a 1/64 turntable turn)
const unsigned long medPressWait = 1000; // in milliseconds, time to wait for med press or 1/2 long press
const bool clockwise = false; // turn motor clockwise or cc?

// VARIABLES
unsigned long pressTime = 0; // time in millis at which button was pressed (0 if not currently pressed)
bool bCameraReady = true; // on short button press: take shot (true) or move turntable (false)
unsigned long stepsPerTurn = 0; // calculated from turnsPerCircle
// debounce
int buttonState = LOW; // official debounced button reading
int lastButtonState = LOW; // previous real reading of button (for debounce)
unsigned long lastDebounceTime = 0; // last time button toggled
unsigned long debounceDelay = 50; // ms

void setup() {

  stepper.setRpm(rpmMotor); // set the motor's rpm
  pinMode(buttonPin, INPUT); // input on buttonPin
  pinMode(irLedPin, OUTPUT); // camera trigger IR LED
  pinMode(camLedPin, OUTPUT); // display LED for camera ready
  pinMode(moveLedPin, OUTPUT); // display LED for move ready

  // determine how many motor microsteps per turn to take (according to turnsPerCircle and gearRatio)
  stepsPerTurn = 409600 / (unsigned long) turnsPerCircle; // 4096 *100 to add 2 decimal place precision
  stepsPerTurn /= (unsigned long) gearRatioX100; // now adjust for gear ratio
  
  if (DEBUG) {
    Serial.begin(9600);
    Serial.print("\n\nserial set up");
    Serial.print("\nstepper pins:");
    for (int i=0; i<4; i++){ Serial.print(" "); Serial.print(stepper.getPin(i));  }
    Serial.print("\nIR LED pin: "); Serial.print(irLedPin);
    Serial.print("\nbutton pin: "); Serial.print(buttonPin);
    Serial.print("\ncamera ready led pin: "); Serial.print(camLedPin);
    Serial.print("\nmove ready led pin: "); Serial.print(moveLedPin);

    Serial.print("\nmotor rpm set to "); Serial.print(stepper.getRpm());
    Serial.print("\nturns per circle: "); Serial.print(turnsPerCircle);
    Serial.print("\ngear ratio (* 100): "); Serial.print(gearRatioX100);
    Serial.print("\nmotor microsteps per turn: "); Serial.print(stepsPerTurn);
    Serial.print("\nwait time for med button press (ms): "); Serial.print(medPressWait);  
    Serial.println();
  }
  
}

void loop() {

  int mode = getMode();
  switch (mode) {
    case 0:{ // stop rotation
      stepper.stop();
      break;
    }
    case 1:{ // take picture, stay still
      stepper.stop();
      triggerCamera();
      break;
     }
    case 2:{ // initiate rotation
      if (stepper.getStepsLeft() == 0){
        stepper.newMove(clockwise, stepsPerTurn);
        if (DEBUG) { Serial.print(" ... initiating new move of steps: "); Serial.print(stepsPerTurn); }
      }
      break;
    }
    case 3:{ // long press rotation - blocking!
      stepper.move(clockwise, (stepsPerTurn/4)); // quarter-turn
      if (DEBUG) { Serial.print(" ... rotating steps: "); Serial.print(stepsPerTurn/4); }
      break;
    }
    default:{ // do nothing
      break;
    }
  }

  stepper.run(); // update stepper (move if moving)

  if (bCameraReady) {
    digitalWrite (camLedPin, true);
    digitalWrite (moveLedPin, false);
  } else {
    digitalWrite (camLedPin, false);
    digitalWrite (moveLedPin, true);
  }

  if (DEBUG) {
    
    switch (mode) {
      case 0: Serial.print("\nSTOP MOVE ... cam ready: "); Serial.print(bCameraReady); break;
      case 1: Serial.print("\nPICTURE ... cam ready: "); Serial.print(bCameraReady); break;
      case 2: Serial.print("\nSTART MOVE ... cam ready: "); Serial.print(bCameraReady); break;
      case 3: Serial.print("\nLONG PRESS MOVE ... cam ready: "); Serial.print(bCameraReady); break;
      default: break;
    }

  }

}

int getMode() {
  // translates button state to modes 0-3
  // 0 : stop rotating
  // 1 : take still picture
  // 2 : begin rotation
  // 3 : long press rotation
  // 4 : no change
  int mode = 4; // default to no change

  bool bPress = debounceRead(buttonPin); // get debounced button input

  // button is pressed 
  if (bPress){
    
    // is it a new button press
    if (pressTime == 0) {
      pressTime = millis(); // save time
      mode = 0; // stop rotation
    }
    else {

      // is it a medium button press
      if (millis() - pressTime >= medPressWait){
        bCameraReady = true; // ready another shot
      }
      
      // is it a long button press
      if (millis() - pressTime >= (medPressWait*2)){
        mode = 3; // start rotating (& continue indefinitely)
      }
    }
  }

  // button is not pressed
  else {
    
    // is it a button release
    if (pressTime > 0) {

      // is it a short press release
      if (millis() - pressTime < medPressWait){
        
        if (bCameraReady) {
          mode = 1; // trigger camera
        } else {
          mode = 2; // start rotation
        }
        bCameraReady = !bCameraReady;
      }

      // or a long or medium press release
      else {
        mode = 0; // stop rotating
      }
    }
    
    pressTime = 0; // reset pressTime
    
  }
  
  return mode; // defaults to 4, no change
}

int debounceRead (int inputPin) {

  int reading = digitalRead(buttonPin);
  
  // if button reading has changed (either noise or press/release)
  if (reading!= lastButtonState) {
    lastDebounceTime = millis(); // reset timer
  }

  // if we've got the same reading long enough to register a new press/release (i.e. debounced)
  if ((millis() - lastDebounceTime) > debounceDelay) {
    
    if (DEBUG && buttonState != reading){
      if (reading == 0) Serial.print(" ... button release");
      else Serial.print(" ... button press");
    }
    
    buttonState = reading; // update official button state

  }
  
  lastButtonState = reading; // update button check
  return buttonState;
}


void triggerCamera() {

  // signal code from:
  // http://controlyourcamera.blogspot.com/2010/01/infrared-controlled-timelapse.html
  
  for(int i=0; i<16; i++) { 
    digitalWrite(irLedPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(irLedPin, LOW);
    delayMicroseconds(10);
   } 
   delayMicroseconds(7330); 
   for(int i=0; i<16; i++) { 
     digitalWrite(irLedPin, HIGH);
     delayMicroseconds(10);
     digitalWrite(irLedPin, LOW);
     delayMicroseconds(10);
   }   
}
