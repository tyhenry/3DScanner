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
    gui->setTheme(new GuiTheme(ofGetWidth()*0.2f));

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
    clockwiseToggle = gui->addToggle("Move Table Clockwise", true);
    autoscanToggle = gui->addToggle("Start Auto-scan", false);
    autoscanLabel = gui->addLabel("Auto-scan Shots Left: ");
    shutterBtn = gui->addButton("Take Shot");
    turnBtn = gui->addButton("Move one Turn");
    rotateBtn = gui->addButton("Hold to Rotate");
    rotateSlider = gui->addSlider("Set Turntable Angle", 0, 360);
    stepLabel = gui->addLabel("Table Step #: ");
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
    serialDeviceDropdown->setLabelColor(ofColor::orange);
    serialBaudDropdown->setStripeColor(green);
    for (int i=0; i<baudRates.size(); i++){
        serialBaudDropdown->getChildAt(i)->setStripeVisible(false);
    }
    serialBaudDropdown->setLabelColor(ofColor::orange);
    scannerConnectBtn->setStripeColor(green);
    scannerConnectBtn->setLabelColor(ofColor::orange);
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
    stepLabel->setStripeColor(red);
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
        // this now happens during update() per !mouseDown check
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
    
    // WATCH FOLDER GUI SETUP
    
    folderGui = new ofxDatGui(gui->getWidth()+10,0);
    int fgW = ofGetWidth()-gui->getWidth()-20;
    folderGui->setTheme(new GuiTheme(fgW));
    
    folderGui->addHeader(":: DROP WATCH FOLDER HERE ::");
    
    folderInput = folderGui->addTextInput("Watch Folder:");
    
    folderGui->update();
    
    folderInput->setStripeColor(ofColor::blueSteel);
    
    // call back for text input
    folderInput->onTextInputEvent(this, &ofApp::newWatchFolderInput);
    
    // calc area available for image drawing
    ofVec2f topLeft(gui->getWidth()+10,folderGui->getHeight()+10);
    float w = ofGetWidth()-10-topLeft.x;
    float h = min(ofGetHeight()-topLeft.y-10, w);
    w = h;
    ofVec2f bottomRight(topLeft.x+w,topLeft.y+h);
    imgArea = ofRectangle(topLeft, bottomRight);
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
    
    // check for new files in watch folder and update ofImage vector
    if (watchFolder.exists()){
        int nNew = loadNewImages();
        if (nNew > 0) {
            // recalc img width/height (square) for given area
            imgWidth = sqrt(imgArea.getArea()/images.size());
            ofLogVerbose("ofApp::update") << "loaded " << nNew << " new images";
        }
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    ofSetColor(50);
    ofDrawRectangle(imgArea);
    
    ofSetColor(255);
    ofVec2f pt = imgArea.getTopLeft();
    for (int i=0; i<images.size(); i++){
        
        if (pt.x + imgWidth > imgArea.width + imgArea.getLeft()) { // if overflow imgArea width
            pt.x = imgArea.getLeft();
            pt.y += imgWidth; // new row
        }
        float imgHeight = (imgWidth/images[i].getWidth())*images[i].getHeight();
        images[i].draw(pt.x, pt.y, imgWidth, imgHeight);
        pt.x += imgWidth;
    }
 
}

//--------------------------------------------------------------
void ofApp::updateGui(){
    
    rpmSlider->setValue(scanner.getRpm());
    numShotsSlider->setValue(scanner.getNumShotsPerRotation());
    waitSlider->setValue(scanner.getWaitAfterShot());
    clockwiseToggle->setChecked(scanner.isClockwise());
    autoscanToggle->setChecked(scanner.isAutoscanning());
    if (!scanner.isMoving()){
        rotateSlider->setValue(scanner.getDegree());
    }
    
    // step label
    string stpLbl = "Table Step #:     ";
    stpLbl += ofToString(scanner.getCurrentStep());
    stpLbl += " / ";
    stpLbl += ofToString(scanner.getNumStepsTurntable());
    stepLabel->setLabel(stpLbl);
    
    // autoscan label (# shots left)
    string asLbl = "Autoscan Shots Left:     ";
    asLbl += ofToString(scanner.getAutoscanShotsLeft());
    asLbl += " / ";
    asLbl += ofToString(scanner.getNumShotsPerRotation());
    autoscanLabel->setLabel(asLbl);
    
    // command output label (last cmdVal received)
    string cmdLbl = "Scanner Output:     ";
    char cmd; unsigned long val;
    if (scanner.getLastCmdValRcvd(&cmd, &val)){
        cmdLbl += ofToString(cmd);
        cmdLbl += ofToString(val);
    }
    commandOutput->setLabel(cmdLbl);
    
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
        serialDeviceDropdown->setLabelColor(ofColor::white);
    }
    else if (e.target==serialBaudDropdown){ // lambda, set serial baud dropdown label
        string lbl = "Baud Rate: ";
        baudRate = baudRates[e.child];
        lbl += ofToString(baudRate);
        serialBaudDropdown->setLabel(lbl);
        serialBaudDropdown->setLabelColor(ofColor::white);
    }
    else {
        ofLogError("ofxDatGui") << "unspecified dropdown target!";
    }
}

//--------------------------------------------------------------
void ofApp::connectScanner(ofxDatGuiButtonEvent e){
    
    // if we're connected to serial, close the connection first
    if (serial.isInitialized() /*&& !scanner.isConnected()*/){
        serial.close();
        scanner.disconnect(); // reset scanner connection, just in case
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
        
        
        // send current values to scanner
        if (scanner.isConnected()){
            scanner.setClockwise(clockwiseToggle->getChecked());
            // use gui callbacks
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
void ofApp::newWatchFolderInput(ofxDatGuiTextInputEvent e){
    
    if (loadWatchFolder(e.target->getText())) {
        folderInput->setLabelColor(ofColor::green);
    } else {
        folderInput->setLabelColor(ofColor::red);
        string fLbl = "No folder at: " + e.target->getText();
        folderInput->setText(fLbl);
    }
}

//--------------------------------------------------------------
bool ofApp::loadWatchFolder(string path){
    
    // test whether file/dir is real
    
    ofFile file(path);
    if (!file.exists()){
        ofLogError("ofApp") << "watch folder load error! - " << path;
        return false;
    }
    
    // load watch folder
    
    if (file.isDirectory()) {        // already dir, load
        watchFolder = ofDirectory(path);
        
    } else {        // find enclosing dir, load
        string dir = file.getEnclosingDirectory();
        watchFolder = ofDirectory(dir);
    }
    
    if (watchFolder.isDirectory()){ // double check
        
        watchFolder.allowExt("jpg"); // allow only jpegs

        watchFolder.listDir(); // list
        watchFolder.sort(); // sort alphabetical
        
        vector <ofFile> files = watchFolder.getFiles(); // get files
        int nFiles = files.size();
        
        ofLogVerbose("ofApp") << "\n  watch folder:\n    " << watchFolder.getAbsolutePath() << "\n  contains " << nFiles << " jpeg files:";
        if (ofGetLogLevel() == OF_LOG_VERBOSE){
            for (int i=0; i<files.size(); i++){
                cout << "    " << files[i].getAbsolutePath() << endl;
            }
        }
        // set folder input text to path
        folderInput->setLabelColor(ofColor::green);
        folderInput->setText(watchFolder.getAbsolutePath());
        
        return true;
    }
    
    ofLogError("ofApp::loadWatchFolder") << "watch folder load error! - " << watchFolder.getAbsolutePath();
    return false;

    
}

//--------------------------------------------------------------
int ofApp::loadNewImages(){
    
    int numNew = 0;
    
    watchFolder.listDir(); // relist
    
    if (loadedFiles.size() < watchFolder.getFiles().size()){ // new image(s)
        
        watchFolder.sort(); // sort alphabetical
        
        for (int w=0; w<watchFolder.getFiles().size(); w++){ // find new
            
            bool loaded = false;
            for (int l=0; l<loadedFiles.size(); l++){
                if (loadedFiles[l] == watchFolder[w])
                    loaded = true; // check against loaded files
            }
            
            if (loaded == false){ // new image, load
                
                ofFile imgFile(watchFolder.getFiles()[w]);
                ofImage img;
                if (img.load(imgFile)){
                    images.push_back(img);
                    loadedFiles.push_back(imgFile);
                    numNew++;
                    
                } else {
                    ofLogError("ofApp::loadNewImages") << "error loading image file: " << imgFile.getAbsolutePath();
                }
            }
        }
    }
    return numNew;
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){
    
    if (folderGui->getHeader()->hitTest(dragInfo.position) || folderInput->hitTest(dragInfo.position)){
        // dragged onto folder gui
        
        int nFiles = dragInfo.files.size();
        if (nFiles > 0){
            loadWatchFolder(dragInfo.files[0]);
        }
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


