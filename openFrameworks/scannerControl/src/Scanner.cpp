//
//  Scanner.cpp
//  scannerControl
//
//  Created by Tyler on 7/30/16.
//
//

#include "Scanner.hpp"

Scanner::Scanner(ofSerial* serialPtr){
    
    commander = Commander(serialPtr); // create serial commander
}

bool Scanner::connect(){
    connected = commander.connect();
    return connected;
}

int Scanner::update(){
    
    commander.update(); // get all new cmds
    
    // run through input queue and return num cmds processed
    int numCmds = 0;
    char cmd; unsigned long val;
    while (commander.getNext(&cmd, &val)){
        numCmds++;
        parse(cmd,val);
        ofLogNotice("Scanner") << "parsing cmd: " << cmd << " val: " << val;
    }
    return numCmds;
}

void Scanner::setClockwise(bool cw){
    
    if (cw)
    commander.send('K',(int)(!cw)); // reversed: table cw == motor ccw
}

void Scanner::autoscan(bool start){
    
    commander.send('A',(int)start); // 1 start, 0 stop
}

void Scanner::setRpm(int motorRpm){
    
    commander.send('R',motorRpm);
}

void Scanner::setNumStepsTurntable(int numSteps){
    
    nStepsTurntable = numSteps;
    commander.send('G',numSteps);
}

void Scanner::setNumShots(int nShots){
    
    commander.send('C',nShots);
}

void Scanner::setWaitAfterShot(int waitSeconds){
    
    commander.send('W', waitSeconds*1000); // cvt to ms
}

void Scanner::takePhoto(){
    
    commander.send('P', 1);
}

void Scanner::turn(){
    
    commander.send('M', 1);
    
}

void Scanner::rotate(){
    commander.send('T',1);
}

void Scanner::rotateTo(float degree){
    
    // convert degree to step #
    degree = abs(degree); // make positive
    unsigned long step = degree/360.0 * (float)nStepsTurntable;
    commander.send('S',step);
    
    ofLogNotice("Scanner") << "moving to degree: " << degree << " - step #: " << step;
}

void Scanner::sendCommand(unsigned char cmd, unsigned long val){
    commander.send(cmd, val);
}

void Scanner::sendCommand(string command){
    commander.send(command);
}

float Scanner::getDegree(){
    return (float)currentStep/(float)nStepsTurntable * 360.0; // calc degree from current step
}

bool Scanner::getLastCmdValRcvd(char* cmd, unsigned long* val){
    if (lastCmdRcv != 0){
        *cmd = lastCmdRcv;
        *val = lastValRcv;
        return true;
    }
    return false;
}


// PRIVATE


bool Scanner::parse(char cmd, unsigned long val){
    
    bool good = true;
    
    if (cmd == 'R') rpm = val;
    else if (cmd == 'M') bMoving = (val == 0) ? 0:1;
    else if (cmd == 'P') bShooting = (val == 0) ? 0:1;
    else if (cmd == 'C') numShotsPerRotation = val;
    else if (cmd == 'K') clockwise = (val == 0) ? 1:0; // reversed (table v. motor)
    else if (cmd == 'A') autoscanShotsLeft = val;
    else if (cmd == 'W') waitSeconds = val/1000;
    else if (cmd == 'G') nStepsTurntable = val;
    else if (cmd == 'S') {
        currentStep = nStepsTurntable - val;
        if (currentStep == nStepsTurntable) currentStep = 0;
        else if (currentStep > nStepsTurntable) {
            // bug in motor code? step # doesn't wrap around total steps if already higher...
            // ideally, don't set gear ratio/total steps after scanner/stepper init
            // hack fix
            val %= nStepsTurntable;
            currentStep = nStepsTurntable-val;
        }
    }
    else if (cmd == 'Q') nCmdsAtArduino = val;
    else good = false;
    
    if (good) {
        lastCmdRcv = cmd;
        lastValRcv = val;
    }
    
    return good;
}


