#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    ofBackground(0);
    //ofSetLogLevel(OF_LOG_VERBOSE);
    
    // SERIAL PREP
    
    // get serial devices
    serial.listDevices(); // list to console
    vector <ofSerialDeviceInfo> devices = serial.getDeviceList(); // save device list

    // setup baud rate choices
    baudRates.push_back(9600); baudRates.push_back(19200); baudRates.push_back(38400);
    baudRates.push_back(57600); baudRates.push_back(74880); baudRates.push_back(115200);
    
    // convert to string vectors for gui
    // devices
    vector <string> deviceStrings;
    for (int i=0; i<devices.size(); i++) { deviceStrings.push_back(devices[i].getDevicePath()); }
    // baudrates
    vector <string> baudStrings;
    for (int i=0; i<baudRates.size(); i++) { baudStrings.push_back(ofToString(baudRates[i])); }
    
    // CREATE SCANNER
    
    scanner = Scanner(&serial); // create scanner with serial ptr
    
    
    // GUI
    
    gui = new ofxDatGui(0,0);
    gui->setTheme(new GuiTheme);

    gui->addHeader(":: SCANNER CONTROLS ::");
    
    // SETUP CONTROLS
    
    ofxDatGuiLabel* setupLabel = gui->addLabel("SETUP");
    serialDeviceDropdown = gui->addDropdown("Serial Device:", deviceStrings);
    serialBaudDropdown = gui->addDropdown("Baud Rate:", baudStrings);
    serialBaudDropdown->setIndex(baudStrings.size()-1); // default choose highest baud rate
    scannerConnectBtn = gui->addButton("Connect to Scanner");
    gearInput = gui->addTextInput("Gear Ratio (Motor:Table)");
    rpmSlider = gui->addSlider("Motor RPM", 7.0, 18.0);
    rpmSlider->setPrecision(0); // int slider
    numShotsSlider = gui->addSlider("Shots per Rotation", 1, 100);
    numShotsSlider->setPrecision(0); // int slider
    turnDegreesLabel = gui->addLabel("^ Degrees per Turn");
    waitSlider = gui->addSlider("Wait for Photo (sec)", 1, 32);
    waitSlider->setPrecision(0); // int slider
    
    serialDeviceDropdown->expand();
    
    // SCAN CONTROLS
    
    gui->addBreak();
    
    ofxDatGuiLabel* scanLabel = gui->addLabel("SCAN");
    
    camReadyLabel = gui->addLabel("Ready to Shoot");
    clockwiseToggle = gui->addToggle("Move Clockwise", true);
    autoscanToggle = gui->addToggle("Start Auto-scan", false);
    autoscanLabel = gui->addLabel("Auto-scan Shots Left: ");
    shutterBtn = gui->addButton("Take Shot");
    turnBtn = gui->addButton("Move one Turn");
    rotateBtn = gui->addButton("Hold to Rotate");
    rotateSlider = gui->addSlider("Set Turntable Angle", 0, 360);
    commandInput = gui->addTextInput("Text Command:");
    commandOutput = gui->addLabel("Scanner Output:");
    
    
    // UPDATE THEME
    
    gui->update();
    
    
    // COMPONENT-LEVEL SETTINGS
    
    // -- SETUP
    
    ofColor green = ofColor::green;
    setupLabel->setLabelColor(green);
    setupLabel->setStripeColor(green);
    
    serialDeviceDropdown->setStripeColor(green);
    for (int i=0; i<deviceStrings.size(); i++){
        serialDeviceDropdown->getChildAt(i)->setStripeVisible(false);
    }
    serialBaudDropdown->setStripeColor(green);
    for (int i=0; i<baudRates.size(); i++){
        serialBaudDropdown->getChildAt(i)->setStripeVisible(false);
    }
    scannerConnectBtn->setStripeColor(green);
    gearInput->setStripeColor(green);
    rpmSlider->setStripeColor(green);
    numShotsSlider->setStripeColor(green);
    turnDegreesLabel->setStripeColor(green);
    turnDegreesLabel->setLabelAlignment(ofxDatGuiAlignment::RIGHT);
    waitSlider->setStripeColor(green);
    
    // -- SCANNER
    
    ofColor red = ofColor::red;
    scanLabel->setLabelColor(red);
    scanLabel->setStripeColor(red);
    
    camReadyLabel->setStripeColor(red);
    camReadyLabel->setLabelAlignment(ofxDatGuiAlignment::CENTER);
    camReadyLabel->setLabelColor(green);
    clockwiseToggle->setStripeColor(red);
    autoscanToggle->setStripeColor(red);
    autoscanLabel->setStripeColor(red);
    shutterBtn->setStripeColor(red);
    turnBtn->setStripeColor(red);
    rotateBtn->setStripeColor(red);
    rotateSlider->setStripeColor(red);
    commandInput->setStripeColor(red);
    commandOutput->setStripeColor(red);
    
    
    // EVENTS
    
    // -- SETUP
    
    gui->onDropdownEvent(this, &ofApp::onDropdownEvent); // select device or baud rate
    
    scannerConnectBtn->onButtonEvent(this, &ofApp::connectScanner); // connect to scanner
    
    gearInput->onTextInputEvent(this, &ofApp::newGearRatioInput);

    rpmSlider->onSliderEvent([&](ofxDatGuiSliderEvent e){ // lambda, set scanner rpm
        scanner.setRpm(rpmSlider->getValue());
    });
    
    numShotsSlider->onSliderEvent([&](ofxDatGuiSliderEvent e){
        // lambda, set scanner num shots/rotation, update turn degrees
        
        scanner.setNumShots(numShotsSlider->getValue());
        
        // update turn degrees label
        float turnDegrees = 360.0/numShotsSlider->getValue();
        string label = "^ Degrees per Turn: " + ofToString(turnDegrees,3);
        turnDegreesLabel->setLabel(label);
    });
    waitSlider->onSliderEvent([&](ofxDatGuiSliderEvent e){ // lambda, set scanner time to wait after shot (in sec)
        scanner.setWaitAfterShot(waitSlider->getValue());
    });
    
    // -- SCANNER
    
    clockwiseToggle->onToggleEvent([&](ofxDatGuiToggleEvent e){ // lambda, set scanner cw/ccw
        scanner.setClockwise(clockwiseToggle->getChecked());
    });
    autoscanToggle->onToggleEvent([&](ofxDatGuiToggleEvent e){ // lamba, start/stop scanner autoscan
        scanner.autoscan(autoscanToggle->getChecked());
    });
    shutterBtn->onButtonEvent([&](ofxDatGuiButtonEvent e){ // lamba, take picture
        scanner.takePhoto();
    });
    turnBtn->onButtonEvent([&](ofxDatGuiButtonEvent e){ // lamba, move one turn
        scanner.turn();
    });
    rotateBtn->onButtonEvent([&](ofxDatGuiButtonEvent e){ // lamba, rotate continously
        // scanner.rotate();
        // this happens during update() with mouseDown check
    });
    rotateSlider->onSliderEvent([&](ofxDatGuiSliderEvent e){ // lambda
        rotationChanged = true;
    });
    commandInput->onTextInputEvent([&](ofxDatGuiTextInputEvent e){ // lambda
        scanner.sendCommand(commandInput->getText());
    });
    
    
    // DEFAULT VALUES
    
    gearInput->setText("1:4");
    rpmSlider->setValue(10);
    numShotsSlider->setValue(24);
    rotateSlider->setValue(0);
    waitSlider->setValue(3);
    
    // FORCE EVENT CALLBACKS
//    gearInput->onFocusLost();
//    rpmSlider->dispatchSliderChangedEvent();
//    numShotsSlider->dispatchSliderChangedEvent();
//    waitSlider->dispatchSliderChangedEvent();
    
    
    // update turn degrees label
//    float turnDegrees = 360.0/numShotsSlider->getValue();
//    string label = "^ Degrees per Turn: " + ofToString(turnDegrees,3);
//    turnDegreesLabel->setLabel(label);
    
    
  //    vector<string> shutterSpeeds = {
  //        "30\"","25\"","20\"","15\"","13\"","10\"","8\"","6\"","5\"","4\"",
  //        "3\"2","2\"5","2\"","1\"6","1\"3","1\"","0\"8","0\"6","0\"5","0\"4","0\"3",
  //        "1/4","1/5","1/6","1/8","1/10","1/13","1/15","1/20","1/25",
  //        "1/30","1/40","1/50","1/60","1/80","1/100","1/125","1/160","1/200","1/250","1/320","1/400"};
  //    shutterMenu = new ofxDatGuiScrollView("Shutter Speed", 10);
  //    gui->onScrollViewEvent(this, &ofApp::onScrollEvent);
    

}

//--------------------------------------------------------------
void ofApp::update(){
    
    if (scanner.isConnected()){
    
        // hold to rotate contiously
        if (rotateBtn->getMouseDown()){
            if (ofGetElapsedTimef() - startRotateTime > waitBetweenRotatePresses){
                scanner.rotate();
                startRotateTime = ofGetElapsedTimef();
            }
        }
        // change in rotation degree + mouse released
        if (rotationChanged && !rotateSlider->getMouseDown()){
            float rotation = rotateSlider->getValue();
            if (rotation == 360.0) { rotateSlider->setValue(0.0); }
            rotationChanged = false;
            // move scanner to rotation degree
            scanner.rotateTo(rotation);
        }
        
        // get all new output from scanner
        if (scanner.update() > 0) { // new data from scanner
            updateGui();
        }
    }
    
}

//--------------------------------------------------------------
void ofApp::draw(){
    
 
}

//--------------------------------------------------------------
void ofApp::updateGui(){
    
    rpmSlider->setValue(scanner.getRpm());
    numShotsSlider->setValue(scanner.getNumShotsPerRotation());
    waitSlider->setValue(scanner.getWaitAfterShot());
    clockwiseToggle->setChecked(scanner.isClockwise());
    autoscanToggle->setChecked(scanner.isAutoscanning());
    rotateSlider->setValue(scanner.getDegree());
    
    // autoscan label (# shots left)
    string asLbl = "Autoscan Shots Left: ";
    asLbl += ofToString(scanner.getAutoscanShotsLeft());
    autoscanLabel->setLabel(asLbl);
    
    // cam ready label
    if (scanner.isMoving() && scanner.isShooting()){
        camReadyLabel->setLabelColor(ofColor::red);
        camReadyLabel->setLabel("Moving and Shooting!");
    } else if (scanner.isMoving()){
        camReadyLabel->setLabelColor(ofColor::red);
        camReadyLabel->setLabel("Moving");
    } else if (scanner.isShooting()){
        camReadyLabel->setLabelColor(ofColor::red);
        camReadyLabel->setLabel("Shooting");
    } else {
        camReadyLabel->setLabelColor(ofColor::green);
        camReadyLabel->setLabel("Ready to Shoot");
    }
    
}

//--------------------------------------------------------------
void ofApp::onDropdownEvent(ofxDatGuiDropdownEvent e){
    
    if (e.target==serialDeviceDropdown) {
        string lbl = "Serial Device: ";
        serialDevice = serialDeviceDropdown->getChildAt(e.child)->getName();
        lbl += serialDevice;
        serialDeviceDropdown->setLabel(lbl);
        //gui->update(); // ? fix
    }
    else if (e.target==serialBaudDropdown){ // lambda, set serial baud dropdown label
        string lbl = "Baud Rate: ";
        baudRate = baudRates[e.child];
        lbl += ofToString(baudRate);
        serialBaudDropdown->setLabel(lbl);
        //gui->update(); // ? fix
    }
    else {
        ofLogError("ofxDatGui") << "unspecified dropdown target!";
    }
}

//--------------------------------------------------------------
void ofApp::connectScanner(ofxDatGuiButtonEvent e){
    
    // if we're connected to serial but not the scanner, close the connection
    if (serial.isInitialized() && !scanner.isConnected()){
        serial.close();
        ofLogNotice("ofSerial") << "closing current connection";
    }
    
    // if we're not already connected and we have a device and baudrate selected
    if (!scanner.isConnected() && serialDevice != "" && baudRate != 0){

        string newLbl = "";
        ofColor scanColor = ofColor::red; ofColor serColor = ofColor::red;
        
        // connect to serial, check if worked
        if (serial.setup(serialDevice,baudRate)) {
            
            serColor = ofColor::green;
            
            // connect to scanner, check if worked
            if (scanner.connect()){
                newLbl = "Scanner Connected";
                scanColor = ofColor::green;
                
            } else newLbl = "Connect to Scanner (SCAN ERR)";
            
        } else newLbl = "Connect to Scanner (SER ERR)";
        
        
        scannerConnectBtn->setLabel(newLbl);
        scannerConnectBtn->setLabelColor(scanColor);
        
        serialBaudDropdown->setLabelColor(serColor);
        serialDeviceDropdown->setLabelColor(serColor);
        
        
        // send default values to scanner
        if (scanner.isConnected()){
            // callbacks
            gearInput->onFocusLost();
            rpmSlider->dispatchSliderChangedEvent();
            numShotsSlider->dispatchSliderChangedEvent();
            waitSlider->dispatchSliderChangedEvent();
        }
    }
}

//--------------------------------------------------------------
void ofApp::newGearRatioInput(ofxDatGuiTextInputEvent e){
    
    // parse gear ratio
    vector <string> tokens = ofSplitString(e.target->getText(),":",true,true); // ignore empty + trim
    
    bool good = false; float gr = 0; int nSteps = 0;
    
    if (tokens.size() == 2) { // good
        
        int motor = ofToInt(tokens[0]);
        int turntable = ofToInt(tokens[1]);
        
        if (motor != 0 && turntable != 0){ // good
            
            gr = (float)turntable/(float)motor; // calc gr as float
            nSteps = 4096 * gr; // calc num motor steps per turntable rotation
            
            if (nSteps > 0 && nSteps <= 32767) { // max arduino range
                
                good = true; // good!
            }
        }
    }
    
    if (good){
        // set gear ratio
        string lbl = e.target->getText() + " - " + ofToString(nSteps) + " steps/rot";
        scanner.setNumStepsTurntable(nSteps);
        e.target->setText(lbl);
        e.target->setLabelColor(ofColor::white);
    } else {
        string lbl = "invalid - " + ofToString(nSteps) + " steps/rot";
        e.target->setText(lbl);
        e.target->setLabelColor(ofColor::red);
    }
    
}


//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
