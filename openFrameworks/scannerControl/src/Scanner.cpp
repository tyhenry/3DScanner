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
    //connected = true;
    return connected;
}

void Scanner::setClockwise(bool cw){

}

void Scanner::autoscan(bool start){
    
}

void Scanner::setRpm(int rpm){
    
}

void Scanner::setNumStepsTurntable(int numSteps){
    
}

void Scanner::setNumShots(int nShots){
    
}

void Scanner::setWaitAfterShot(int waitSeconds){
    
}

void Scanner::takePhoto(){
    
}

void Scanner::turn(){
    
}

void Scanner::rotate(){
    cout << "rotating..." << endl;
}

void Scanner::rotateTo(float degree){
    cout << "rotating to " << degree << endl;
}

void Scanner::sendCommand(string command){
    cout << command << endl;
}




