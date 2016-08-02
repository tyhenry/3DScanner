#pragma once
#include <CheapStepper.h>

class Scanner {

public:
  Scanner(){
    stepper = CheapStepper(stepperPins[0], stepperPins[1], stepperPins[2], stepperPins[3]);
    calcStepsPerTurn(); // init based on default values
  }
  Scanner(int p1, int p2, int p3, int p4) {
    stepperPins[0] = p1;
    stepperPins[1] = p2;
    stepperPins[3] = p3;
    stepperPins[4] = p4;
    Scanner();
  }

  void update(); // call every loop()!!!
  void setIrLedPin(int pin){ /* sets up infrared led cam trigger */ irLedPin = pin; pinMode(irLedPin, OUTPUT); } // must be called at least once!!
  void setCamLedPin(int pin){ /* sets up camera ready status led */ camLedPin = pin; pinMode(camLedPin, OUTPUT); }

  void startAutoscan();
  void stopAutoscan(); // cancel autoscan
  void takePhoto();
  void turn(); // intiate one turn
  void rotateTurntable(); // rotate continuously

  /* set stepper motor RPM */
  void setMotorRpm(int r){ 
    stepper.setTotalSteps(4096); // reset so as not to throw off rpm calc
    stepper.setRpm(r);
    stepper.setTotalSteps(turntableRotationSteps); // set back to original gear ratio
  }
  /* poll stepper RPM */
  int getMotorRpm(){     
    stepper.setTotalSteps(4096); // reset so as not to throw off rpm calc
    int rpm = stepper.getRpm();
    stepper.setTotalSteps(turntableRotationSteps); // set back to original gear ratio
    return rpm;
  }
  int getTurntableRpm(){ return stepper.getRpm(); }
  
  /* set # turns per circle (how many photos/rev?), recalcs stepsPerTurn */ 
  void setTurnsPerCircle(int turns){ turnsPerCircle = turns; calcStepsPerTurn(); }
  int getTurnsPerCircle() { return turnsPerCircle; }

  /* set rotation direction */
  void setClockwise(bool cw) {  bClockwise = cw; }
  bool getClockwise() { return bClockwise; }

  /* set time (ms) to wait after photo trigger before move */ 
  void setWaitAfterPhoto(unsigned long wait) { waitAfterPhoto = wait; }
  unsigned long getWaitAfterPhoto() { return waitAfterPhoto; }

  /* get whether camera is ready to shoot (e.g. moves done, not taking pic) */
  bool isCameraReady() { return (!bMoving && !bShooting); }
  bool isMoving() { return bMoving; }
  bool isShooting() { return bShooting; }

  /* return # moves left in autoscan */
  int getAutoscanMovesLeft() { return autoscanMovesLeft; }
  unsigned long getStepsPerTurn() { return stepsPerTurn; }

  /* set # of steps in turntable rotation, i.e. per motor:turntable gear ratio */
  void setTurntableRotationSteps (int steps);
  int getTurntableRotationSteps () { return turntableRotationSteps; }
  
  /* set # of stepper steps to take if manually rotating turntable */
  void setRotateTurntableSteps (int steps) { rotateTurntableSteps = steps; }
  int getRotateTurntableSteps() { return rotateTurntableSteps; }

  void moveToStep(int stepPos) {
    forceCameraReady(); // cancel move and wait for photo to finish, if any
    stepper.moveTo(bClockwise, stepPos);
  }
  int getStepperPos() { return stepper.getStep(); }

  void forceCameraReady(); // cancels move and finishes photo if any, then bMoving & bShooting = false

  /* return pin #s */
  int getIrLedPin() { return irLedPin; }
  int getCamLedPin() { return camLedPin; }
  int getStepperPin(int p) { return stepper.getPin(p); }

  CheapStepper stepper;

private:

  void calcStepsPerTurn(); // turnsPerCircle -> stepsPerTurn
  void triggerCamera(); // send IR code to camera
  void continueAutoscan(); // continue autoscanning

  int stepperPins[4] = {8,9,10,11}; // 8-11 <--> ULN2003 IN1-IN4

  int irLedPin = 12; // sends IR trigger code to camera
  int camLedPin = 0; // notifiies when camera ready, if 0, don't write to it

  int rpmMotor = 10; // motor RPM (ideal 8 - 20) (limit 6-24) ... at 10RPM motor with 1:4 motor:turntable gear ratio, turntable spins at 2.5 RPM
  int turnsPerCircle = 64; // how many moves/pictures to perform for one turntable rotation
  bool bClockwise = true; // turn cw or ccw?
  unsigned long waitAfterPhoto = 3000; // (ms) time to wait after triggering camera before starting new move (shutter speed depdendant)
  int rotateTurntableSteps = 64; // how many steps to move if manually rotating turntable
  int turntableRotationSteps = 4096; // number of steps in turntable rotation

  bool bShooting = false; // is taking picture?
  bool bMoving = false; // is moving?
  unsigned long stepsPerTurn = 0; // calculated from turnsPerCircle
  int autoscanMovesLeft = 0; // how many moves/photos left in full rotation autoscan (0 when not autoscanning)

  unsigned long photoStart = 0; // saves millis() time when last photo triggered

};


// -----------------
// function definitions:
// -----------------


// -----------------
// PUBLIC API
// -----------------

void Scanner::update() {

  if (autoscanMovesLeft > 0) { // autoscanning
    continueAutoscan();
    return;
  }

  // not autoscanning

  if (!bMoving && !bShooting) {
    if (camLedPin != 0){ digitalWrite(camLedPin, HIGH); } // notify cam ready
  }
  
  else if (bMoving) { // mid-move?

    stepper.run();// update stepper
    
    if (stepper.getStepsLeft() == 0) { // is move done?
      delay(100); // wait 1/10s to prevent turntable jitter
      bMoving = false;
    }
    if (camLedPin != 0) { digitalWrite(camLedPin, LOW); } // cam not ready
  }

  else if (bShooting) { // mid-photo?

    if (millis() - photoStart > waitAfterPhoto) { // is photo done?
      bShooting = false; 
    }
    if (camLedPin != 0) { digitalWrite(camLedPin, LOW); } // cam not ready
  }

}

void Scanner::startAutoscan() {
  
  if (autoscanMovesLeft > 0){ return; } // do nothing if autoscanning already
  
  stopAutoscan(); // reset (if moving, cancels - if shooting, waits for photo)
  autoscanMovesLeft = turnsPerCircle; // init autoscan
}

void Scanner::stopAutoscan() { // cancel autoscan
  forceCameraReady(); // cancels move and finishes photo, if any
  autoscanMovesLeft = 0;
}

void Scanner::takePhoto() { // trigger photo
  if (bShooting) { // mid-photo?
    unsigned long wait = 0; // calc time to wait for photo to finish
    if (millis() - photoStart < waitAfterPhoto) { wait = waitAfterPhoto - (millis() - photoStart); }
    delay(wait);
  }
  triggerCamera();
  photoStart = millis();
  bShooting = true;
}

void Scanner::turn() { // intiate one turn
  stepper.newMove(bClockwise, stepsPerTurn);
  bMoving = true;
} 
void Scanner::rotateTurntable() { // rotate continuously
  stepper.move(bClockwise, rotateTurntableSteps); // blocking move (should be quick so scanner is responsive)
} 

void Scanner::forceCameraReady() {
  if (bMoving) { // mid-move?
    stepper.stop(); // cancel current move
    delay(100); // wait 1/10s to prevent turntable jitter
  }
  if (bShooting) { // mid-photo?
    unsigned long wait = 0; // calc time to wait for photo to finish
    if (millis() - photoStart < waitAfterPhoto) { wait = waitAfterPhoto - (millis() - photoStart); }
    delay(wait);
  }
  bMoving = false; bShooting = false;
}

void Scanner::setTurntableRotationSteps(int steps) {
  
  turntableRotationSteps = steps;
  stepper.setTotalSteps(steps);
  calcStepsPerTurn();
}


// -----------------
// PRIVATE functions
// -----------------

void Scanner::calcStepsPerTurn() {
  // determine how many motor microsteps per turn to take (according to turnsPerCircle)
  unsigned long stepsPerTurnX100 = (unsigned long) turntableRotationSteps * 100 / (unsigned long) turnsPerCircle; // 4096 *100 to add 2 decimal place precision
  stepsPerTurn = stepsPerTurnX100 / 100; // make int
}

void Scanner::triggerCamera() {

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
void Scanner::continueAutoscan() { // continue autoscan

    // take a photo
    if (!bMoving && !bShooting) { // not moving + not shooting = ready to shoot
      if (camLedPin != 0) digitalWrite(camLedPin, HIGH); // notify
      takePhoto();
    }

    // or initiate move
    else if (bShooting) { // mid-shot?

      if (camLedPin != 0) { digitalWrite(camLedPin, LOW); } // cam not ready
      if (millis() - photoStart > waitAfterPhoto) { // if we've waited long enough for photo to finish

        bShooting = false;
        autoscanMovesLeft--; // decrement # moves left
        turn(); // initiate next turn
      }
      
    }

    // or continue move
    else if (bMoving) { // mid-move?

      stepper.run(); // update stepper
      if (camLedPin != 0) { digitalWrite(camLedPin, LOW); } // cam not ready
      if (stepper.getStepsLeft() == 0) { // check if current move done
        delay (100); // delay 1/10s to account for turntable jitter
        bMoving = false;
      }
    }

}


