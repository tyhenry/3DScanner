const unsigned long medPressWait = 1000; // time (ms) to wait before registering med button press or 1/2 long press
// medium press -->> set camera ready
// long press (hold) -->> manually rotate turntable

unsigned long pressTime = 0; // time (millis()) at which button was pressed (0 if not currently pressed)

int debounceRead(int inputPin);

int getButtonMode() {
  
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


// Debounce Button

int buttonState = LOW; // official debounced button reading
int lastButtonState = LOW; // previous real reading of button (for debounce)
unsigned long lastDebounceTime = 0; // last time button toggled
unsigned long debounceDelay = 50; // ms

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
