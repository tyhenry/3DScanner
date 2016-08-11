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
    guiTheme = new GuiTheme(ofGetWidth()*0.2f);
    gui->setTheme(guiTheme);

    gui->addHeader(":: SCANNER CONTROLS ::");
    gui->getHeader()->setDraggable(false);
    
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
    folderGui->getHeader()->setDraggable(false);
    
    folderInput = folderGui->addTextInput("Watch Folder:");
    imgSlider = folderGui->addSlider("Image to Display",0,0);
    imgSlider->setPrecision(0); // int
    animSpeedSlider = folderGui->addSlider("Animation Speed (fps)",0.5,24.0);
    animSpeedSlider->setPrecision(1); // 0.1 resolution on float slider
    animPauseSlider = folderGui->addSlider("Animation Pause Before Loop (sec)",0,10);
    animPauseSlider->setPrecision(1); // 0.1 res
    
    folderGui->update();
    
    folderInput->setStripeColor(ofColor::blueSteel);
    imgSlider->setStripeColor(ofColor::blueSteel);
    animSpeedSlider->setStripeColor(ofColor::blueSteel);
    animSpeedSlider->setValue(4);
    animPauseSlider->setValue(1);
    
    // call backs
    folderInput->onTextInputEvent(this, &ofApp::newWatchFolderInput);
    imgSlider->onSliderEvent([&](ofxDatGuiSliderEvent e){
        imgIdx = e.target->getValue()-1;
    });
    animSpeedSlider->onSliderEvent([&](ofxDatGuiSliderEvent e){ // lamba, round to nearest 0.5
        float nearestHalf = floor((e.target->getValue()*2)+0.5)/2;
        e.target->setValue(nearestHalf);
        animSwitchWait = 1/e.target->getValue();
    });
    animPauseSlider->onSliderEvent([&](ofxDatGuiSliderEvent e){
        float nearestHalf = floor((e.target->getValue()*2)+0.5)/2;
        e.target->setValue(nearestHalf);
    });
    
    // calc area available for image drawing
    
    resizeImgAreas();
    
    // load font
    font = ofxSmartFont::add(guiTheme->font.file,8);
    
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
            imgIdx = images.size()-1; // set imgIdx to newest in vector
            imgSlider->setMax(images.size());
            imgSlider->setMin(1);
            imgSlider->setValue(images.size());
            ofLogVerbose("ofApp::update") << "loaded " << nNew << " new images";
        }
        // run animation
        if (ofGetElapsedTimef()-animSwitchTime >= animSwitchWait){
            bool switchAnim = true;
            if (animFrame == images.size()-1){ // last image, so pause
                if (!(ofGetElapsedTimef()-animSwitchTime >= animPauseSlider->getValue())) switchAnim = false; // not yet time to switch
            }
            // switch frame
            if (switchAnim) {
                if (++animFrame >= images.size()) animFrame = 0; // increment or wrap frame to 0
                animSwitchTime = ofGetElapsedTimef();
            }
        }
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    ofSetColor(50);
    ofDrawRectangle(imgArea);
    ofDrawRectangle(animArea);
    
    ofSetColor(255);
    
    if (images.size() > 0){
        
        // draw last image in images
        float iW = images[imgIdx].getWidth();
        float iH = images[imgIdx].getHeight();
        // scale
        if (iW >= iH){
            iH *= imgArea.width/iW;
            iW = imgArea.width;
        } else {
            iW *= imgArea.height/iH;
            iH = imgArea.height;
        }
        images[imgIdx].draw(imgArea.getTopLeft(),iW,iH);
        
        // label
        string imgLbl = "Image: " + loadedFiles[imgIdx].getFileName();
        font->draw(imgLbl, imgArea.getBottomLeft().x, imgArea.getBottomLeft().y+20.0);
        
        // draw animation
        if (animFrame < images.size()){ // safety
            float hA = (animArea.width/images[animFrame].getWidth())*images[animFrame].getHeight();
            images[animFrame].draw(animArea.getTopLeft(),animArea.width,hA);
            
            // label
            font->draw(loadedFiles[animFrame].getFileName(), animArea.getLeft(), animArea.getBottom()+20.0);
            string animFrameLbl = "Animation frame: " + ofToString(animFrame+1) + " / " + ofToString(images.size());
            ofVec2f animFrameLblPos(animArea.getRight() - font->width(animFrameLbl), animArea.getBottom()+20);
            font->draw(animFrameLbl, animFrameLblPos.x, animFrameLblPos.y);
            
        } else {
            ofLogError("ofApp::draw") << "calling animation frame " << animFrame << " but only " << images.size() << " images loaded!";
        }
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
        
        // reset images vector & animation stuff
        loadedFiles.clear();
        images.clear();
        animSwitchTime = ofGetElapsedTimef();
        animFrame = 0;
        
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
    } else if (loadedFiles.size() > watchFolder.getFiles().size()) { // img delete, reload all
        loadedFiles.clear();
        images.clear();
        animSwitchTime = ofGetElapsedTimef();
        animFrame = 0;
        numNew = loadNewImages();
    }
    return numNew;
}

//--------------------------------------------------------------
void ofApp::resizeImgAreas(){
    
    // newest img
    ofVec2f topLeftImg(gui->getWidth()+10,folderGui->getHeight()+10);
    float imgH = gui->getPosition().y+gui->getHeight()-topLeftImg.y;
    float imgW = (ofGetWidth()-topLeftImg.x-20)*0.5;
    imgH = min(imgH, imgW); // fit
    imgW = imgH;
    ofVec2f bottomRightImg(topLeftImg.x+imgW,topLeftImg.y+imgW); // square
    imgArea = ofRectangle(topLeftImg,bottomRightImg);
    
    // animation
    ofVec2f topLeftAnim(imgArea.getRight()+10,imgArea.getTop());
    ofVec2f bottomRightAnim(topLeftAnim.x+imgArea.width,topLeftAnim.y+imgArea.height);
    animArea = ofRectangle(topLeftAnim,bottomRightAnim);
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
void ofApp::windowResized(int w, int h){
    
    
    // resize folder gui
    folderGui->setWidth(w-folderGui->getPosition().x-10.0);
    
    // resize img displays
    resizeImgAreas();
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
void ofApp::gotMessage(ofMessage msg){

}


