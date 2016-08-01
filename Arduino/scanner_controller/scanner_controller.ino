#include <CheapStepper.h>

// ADJUSTABLE SETTINGS

const bool DEBUG = false; 
// true turns on Serial printing to Arduino IDE console - do not use with oF app, only for hardware debugging

CheapStepper stepper (8,9,10,11); // pins connected to IN1-IN4 on ULN2003
const int irLedPin = 12; // infrared led pin for camera trigger
const int buttonPin = 2; // button press input pin
const int camLedPin = 13; // shows 'cam ready' mode after a move


int turnsPerCircle = 64; // this is how many shots we want to take per revolution
int gearRatioX100 = 25; // gear ratio of motor:turntable - 1:4 = 0.25 (* 100 = 25)
int rpmMotor = 10; // 10 RPM for motor - at 1:4 gear ratio, turntable spins at 2.5 RPM
bool clockwise = false; // turn motor clockwise or cc?
unsigned long waitBetweenMoves = 3000; // (ms) to wait between move + photo


// VARIABLES

bool bCameraReady = true; // on short button press: take shot (true) or move turntable (false)
unsigned long stepsPerTurn = 0; // calculated from turnsPerCircle
int autoScanMovesLeft = 0; // tracks number of moves left in autoscan (0 if not autoscanning)
unsigned long moveStart = 0; // millis() time value when last move was run

unsigned long calcStepsPerTurn();
void triggerCamera();
void runAutoscan();

#include "Button.h"
#include "Commands.h"

void setup() {

  Serial.begin(115200);
  while (!Serial){} // wait for serial connection

  stepper.setRpm(rpmMotor); // set the motor's rpm
  pinMode(buttonPin, INPUT); // input on buttonPin
  pinMode(irLedPin, OUTPUT); // camera trigger IR LED
  pinMode(camLedPin, OUTPUT); // display LED for camera ready

  calcStepsPerTurn(); // turns per circle -> steps per turn
  
  if (DEBUG) {
    Serial.print("\n\nserial set up");
    Serial.print("\nstepper pins:");
    for (int i=0; i<4; i++){ Serial.print(" "); Serial.print(stepper.getPin(i));  }
    Serial.print("\nIR LED pin: "); Serial.print(irLedPin);
    Serial.print("\nbutton pin: "); Serial.print(buttonPin);
    Serial.print("\ncamera ready led pin: "); Serial.print(camLedPin);

    Serial.print("\nmotor rpm set to "); Serial.print(stepper.getRpm());
    Serial.print("\nturns per circle: "); Serial.print(turnsPerCircle);
    Serial.print("\ngear ratio (* 100): "); Serial.print(gearRatioX100);
    Serial.print("\nmotor microsteps per turn: "); Serial.print(stepsPerTurn);
    Serial.print("\nwait time for med button press (ms): "); Serial.print(medPressWait);  
    Serial.println();
  }
  
}

void loop() {

  // 1. run commands from queue
  // 2. apply serial-based settings
  // 3. check button mode
  // 4. button overrides serial-based mode if needed

  runCommands();

  if (autoScanMovesLeft > 0){
    // if we've waited long enough
    if (millis() - moveStart > waitBetweenMoves){
      if (bCameraReady) {
        // shoot picture
        triggerCamera();
        bCameraReady = true;
      }
      else {
        // run autoscan move
        stepper.move(clockwise, stepsPerTurn);
        moveStart = millis();
        bCameraReady = false;
      }
    }
  }
  
  int mode = getButtonMode();
  
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
  } else {
    digitalWrite (camLedPin, false);
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


void runAutoscan() {

  if (autoScanMovesLeft > 0) {
    stepper.stop(); // cancel previous autoscan
  }
  autoScanMovesLeft = turnsPerCircle;
  
}



unsigned long calcStepsPerTurn(){

  // determine how many motor microsteps per turn to take (according to turnsPerCircle and gearRatio)
  stepsPerTurn = 409600 / (unsigned long) turnsPerCircle; // 4096 *100 to add 2 decimal place precision
  stepsPerTurn /= (unsigned long) gearRatioX100; // now adjust for gear ratio

  return stepsPerTurn;
}


void triggerCamera() {

  // IR signal code from:
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



