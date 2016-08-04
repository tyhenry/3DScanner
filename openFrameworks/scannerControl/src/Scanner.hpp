//
//  Scanner.hpp
//  scannerControl
//
//  Created by Tyler on 7/30/16.
//
//

#pragma once
#include "ofMain.h"
#include "Commander.hpp"

class Scanner {
public:
    
    Scanner(){}
    Scanner(ofSerial* serialPtr);
    
    bool connect();
    
    void setClockwise(bool cw);
    void autoscan(bool start); // false for stop
    void startAutoscan() { autoscan(true); }
    void stopAutoscan() { autoscan(false); }
    void setRpm(int rpm);
    void setNumStepsTurntable(int numSteps); // # motor steps in 1 turntable rotation
    void setNumShots(int nShots);
    void setWaitAfterShot(int waitSeconds); // in sec
    void takePhoto();
    void turn();
    void rotate();
    void rotateTo(float degree);
    void sendCommand(string command);
    
    bool isConnected() { return connected; }
    
private:
    
    int serialIdx = 0;
    Commander commander; // serial I/O parsing
    bool connected = false;
    
};