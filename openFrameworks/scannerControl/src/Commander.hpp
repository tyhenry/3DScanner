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
    
    struct cmdVal {
        char cmd = 0;
        unsigned long val = 0;
    };
    
    Commander(){
        memset(buf,0,sizeof(buf)); // clear buffer
    }
    Commander(ofSerial* serialPtr);
    void setSerial (ofSerial* serialPtr) { serial = serialPtr; }
    
    bool connect(); // send/get handshake, true if success
    bool isConnected() { return connected; }
    
    int update(); // returns num of new cmds added to queue
    
    bool send(unsigned char cmd, unsigned long val);
    bool send(string command); // send chars of string with custom end char
    
    cmdVal getNext(); // get next using struct
    bool getNext(char* cmd, unsigned long *val); // get next ptr style
    
    int getNumCmdsQueued() { return cmdQueue.size(); } // # cmds in queue
    
    void setEndChar(unsigned char ec) { endChar = ec; }
        // set char to use as end of msg
    
    void enableLog(bool enable = true){ logging = enable; }
    
    
private:
    
    cmdVal cvtBufToCmdVal();
    // tries to cvt buffer to cmdVal, returns empty cmdVal on fail
    
    ofSerial* serial;
    int serialIdx = 0;
    deque<cmdVal> cmdQueue; // fifo
    deque<cmdVal> outQueue; // fifo
    
    unsigned char buf[11];
    int bufLen = 0; // tracks num of items in buffer
    unsigned char endChar = '\n';
    
    bool connected = false;
    bool logging = false;
};