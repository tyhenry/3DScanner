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
    int update();
    
    void setClockwise(bool cw);
    void autoscan(bool start); // false for stop
    void startAutoscan() { autoscan(true); }
    void stopAutoscan() { autoscan(false); }
    void setRpm(int motorRpm);
    void setNumStepsTurntable(int numSteps); // # motor steps in 1 turntable rotation
    void setNumShots(int nShots);
    void setWaitAfterShot(int waitSeconds); // in sec
    void takePhoto();
    void turn();
    void rotate();
    void rotateTo(float degree);
    void sendCommand(unsigned char cmd, unsigned long val);
    void sendCommand(string command);
    
    int getRpm() { return rpm; }
    bool isMoving() { return bMoving; }
    bool isShooting() { return bShooting; }
    unsigned long getStepsPerTurn() { return stepsPerTurn; }
    int getNumShotsPerRotation() { return numShotsPerRotation; }
    bool isClockwise() { return clockwise; }
    bool isAutoscanning() { if (autoscanShotsLeft > 0) return true; else return false; }
    int getAutoscanShotsLeft() { return autoscanShotsLeft; }
    int getWaitAfterShot() { return waitSeconds; }
    int getNumCmdsAtArduino() { return nCmdsAtArduino; }
    unsigned long getCurrentStep() { return currentStep; }
    unsigned long getNumStepsTurntable() { return nStepsTurntable; }
    float getDegree();
    bool getLastCmdValRcvd(char* cmd, unsigned long* val);
    
    bool isConnected() { return connected; }
    void disconnect() { connected = false; }
    
    
private:
    
    bool parse(char cmd, unsigned long val);
    
    int serialIdx = 0;
    Commander commander; // serial I/O parsing
    bool connected = false;
    
    int numShotsTaken = 0;
    
    unsigned long nStepsTurntable = 1;
    unsigned long currentStep = 0;
    int rpm = 0;
    bool bMoving = false;
    bool bShooting = false;
    unsigned long stepsPerTurn = 0;
    int numShotsPerRotation = 0;
    bool clockwise = true;
    int autoscanShotsLeft = 0;
    int waitSeconds = 0;
    int nCmdsAtArduino = 0; // tracks number of unprocessed cmds in arduino's cmdQueue
    
    char lastCmdRcv = 0; // last cmd received
    unsigned long lastValRcv = 0; // last val received
    
};

/*
 valid cmds:
 'R' = set RPM (val 0: report)
 'M' = move one turn (val 0: report is moving)
 'P' = take picture (val 0: report is shooting)
 'T' = rotate turntable (vals - 0: report # steps per rotation, 1: rotate, other: set # steps per rotation & rotate)
 'C' = set turns per circle (val 0: report current)
 'K' = set clockwise direction (vals - 0: report current, 1: set cw, 2: set ccw)
 'A' = run autoscan (vals - 0: report autoscan move left, 1: run, 2: stop)
 'W' = set wait (ms) after photo before next move (val 0: report current)
 'G' = set turntable total steps (val 0: report current)
 'I' = report step position
 'S' = move to step
 'D' = move to degree
 'Q' = flush cmdQueue (0: report # cmds in queue, other: flush)
 */