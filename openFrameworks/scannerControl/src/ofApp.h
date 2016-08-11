#pragma once

#include "ofMain.h"
#include "ofxDatGui.h"
#include "Scanner.hpp"

class GuiTheme : public ofxDatGuiTheme {
public:
    
    GuiTheme(int width = 400) {
        font.size = 8;
        layout.width = width;
        layout.labelWidth = 180;
        layout.upperCaseLabels = false;
        layout.breakHeight = 10.0f;
        init();
    }
};

class ofApp : public ofBaseApp{

public:
    void setup();
    void update();
    void draw();
    
    void updateGui(); // updates gui based on scanner numbers
    void onDropdownEvent(ofxDatGuiDropdownEvent e);
    void connectScanner(ofxDatGuiButtonEvent e);
    void newGearRatioInput(ofxDatGuiTextInputEvent e);
    
    void newWatchFolderInput(ofxDatGuiTextInputEvent e);
    bool loadWatchFolder(string folderPath);
    
    int loadNewImages();
    void resizeImgAreas();
 
    //void setTurnDegreesLabel();
    
    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void mouseEntered(int x, int y);
    void mouseExited(int x, int y);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    
    ofSerial serial;
    string serialDevice = ""; // saves serial path choice
    vector <int> baudRates;
    int baudRate = 0;
    Scanner scanner;
    
    float startRotateTime = 0;
    float waitBetweenRotatePresses = 0.1;
    bool rotationChanged = false;
    
    // GUI elements
    
    ofxDatGui* gui;
    GuiTheme* guiTheme;
    ofxDatGuiDropdown* serialDeviceDropdown; // list serial devices
    ofxDatGuiDropdown* serialBaudDropdown; // list baud rates
    ofxDatGuiButton* scannerConnectBtn; // connect to scanner @ serial (device, baud)
    ofxDatGuiSlider* rpmSlider;
    ofxDatGuiTextInput* gearInput;
    ofxDatGuiSlider* numShotsSlider;
    ofxDatGuiLabel* turnDegreesLabel;
    ofxDatGuiSlider* waitSlider;
    ofxDatGuiLabel* camReadyLabel;
    ofxDatGuiToggle* clockwiseToggle;
    ofxDatGuiToggle* autoscanToggle;
    ofxDatGuiLabel* autoscanLabel;
    ofxDatGuiButton* shutterBtn;
    ofxDatGuiButton* turnBtn;
    ofxDatGuiButton* rotateBtn;
    ofxDatGuiSlider* rotateSlider;
    ofxDatGuiLabel* stepLabel;
    ofxDatGuiTextInput* commandInput;
    ofxDatGuiLabel* commandOutput;
    
    // Watch folder GUI
    
    ofxDatGui* folderGui;
    ofxDatGuiTextInput* folderInput;
    ofxDatGuiSlider* imgSlider;
    ofxDatGuiSlider* animSpeedSlider;
    ofxDatGuiSlider* animPauseSlider;
    
    ofDirectory watchFolder;
    vector <ofFile> loadedFiles;
    vector <ofImage> images;
    ofRectangle imgArea, animArea; // latest image and looping animation
    int imgIdx = 0; // current img to show
    float animSwitchTime = 0; // time of last frame switch
    float animSwitchWait = 0.25; // wait time in s between frames (fps based)
    int animFrame = 0; // idx of images - frame of animation
    shared_ptr<ofxSmartFont> font;
		
};
