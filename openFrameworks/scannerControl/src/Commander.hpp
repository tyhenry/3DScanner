//
//  Commander.hpp
//  scannerControl
//
//  Created by Tyler on 8/3/16.
//
//

#pragma once
#include "ofMain.h"

class Commander {
    
public:
    
    Commander(){}
    Commander(ofSerial* serialPtr);
    void setSerial (ofSerial* serialPtr) { serial = serialPtr; }
    
private:
    
    ofSerial* serial;
    int serialIdx = 0;
    
};