//
//  Commander.cpp
//  scannerControl
//
//  Created by Tyler on 8/3/16.
//
//

#include "Commander.hpp"

Commander::Commander(ofSerial* serialPtr){
    
    setSerial(serialPtr); // connection should already be established
    Commander();
}

bool Commander::connect(){
    
    connected = false;
    
    // flush serial
    if (serial->isInitialized()){ // we have serial
        
        // pause for 3 sec to make sure Arduino is ready
        float time = ofGetElapsedTimef();
        while (ofGetElapsedTimef() - time < 3){ update(); }
        
        //serial->flush(true,true); // flush in/out
        cmdQueue.clear();
        outQueue.clear();
        
        // send handshake
        send('H',1);
        
        // pause for 1 sec
        time = ofGetElapsedTimef();
        ofLogVerbose("Commander") << "waiting for scanner response...";
        while (ofGetElapsedTimef() - time < 0.5){ /* wait */ }
        
        // check serial for handshake response
        int newCmds = update(); // get all new cmds on serial
        ofLogVerbose("Commander") << "read " << newCmds << " cmds after handshake";
        
        for (int i=0; i<newCmds; i++){
            
            if (cmdQueue[i].cmd == 'H' && cmdQueue[i].val == 1){
                
                connected = true; // success!
                cmdQueue.erase(cmdQueue.begin()+i);
                ofLogNotice("Commander") << "connected to scanner";
                break;
            }
        }
        
    }
    
    return connected;
}

int Commander::update(){
    
    // parse incoming bytes
    int newCmds = 0;
    unsigned long numBytesRead = 0;
    
    if (serial->available() > 0){
        ofLogVerbose("Commander") << "Reading " << serial->available() << " bytes...";
    }
    
    while (serial->available() > 0){
        
        // write to buffer until we reach max buffer length or get an endChar
        
        unsigned char c = serial->readByte(); // get in byte as char
        if (ofGetLogLevel() == OF_LOG_VERBOSE) cout << c;
        
        numBytesRead++;
        
        if (c == endChar) { // if we've reached an endChar
            
            ofLogVerbose("Commander") << "read: " << buf;
            
            // try to convert buffer to cmd/val pair
            
            cmdVal cv = cvtBufToCmdVal();   // convert #bytes in buffer to cmdVal (char and unsigned long)
            
            if (cv.cmd != 0){
                cmdQueue.push_back(cv); // valid, add to queue
                newCmds++;
                ofLogVerbose("Commander") << "read cmd: " << cv.cmd << " val: " << cv.val << " - # cmds in queue: " << cmdQueue.size();
            }
            
            // clear buffer, start fresh
            memset(buf,0,sizeof(buf));  // init to 0
            bufLen = 0;
            
        }
        
        else { // not an end char, save char to buffer
            
            if (bufLen == sizeof(buf)) {   // if buffer overflowing
                
                // report error to console
                ofLogError("Commander") << "parse buffer overflow at " << bufLen+1 << " bytes (too many bytes, no end flag)";
                
                // clear buffer, start fresh
                memset(buf,0,sizeof(buf));  // init to 0
                bufLen = 0;
                
                // read serial until we hit an endChar to clear junk
                
                while (serial->available() > 0 && ((unsigned char)serial->readByte() != endChar)){ /* loop */ }
                
            } else { // room in buffer, save it
                
                // add to buffer
                buf[bufLen++] = c;  // increment buf len after copy
            }
            
        }
    }
    //if (numBytesRead> 0) cout << endl;
    return newCmds;
}

bool Commander::send(unsigned char cmd, unsigned long val){
    
    bool wrote = false;
    
    if (serial->isInitialized()){
        
        wrote = serial->writeByte(cmd); // write command (success, wrote = true)
        
        string valStr = ofToString(val); // cvt val to string
        int valLen = valStr.length();
        
        // write val char by char
        for (unsigned int i=0; i<valLen; i++){
            wrote = serial->writeByte(valStr.at(i)) ? wrote : false;
        }
        
        wrote = serial->writeByte(endChar) ? wrote : false; // end char
    }
    
    if (wrote) {
        // report success to console
        ofLogNotice("Commmander") << "sent cmd: " << cmd << " val: " << val;
    } else {
        // report error to console
        ofLogError("Commander") << "failed to send cmd: " << cmd <<  " val: " << val;
    }
    return wrote; // false if any chars failed to send
}

bool Commander::send(string command){
    
    bool wrote = false;
    int len = command.length();
    
    if (serial->isInitialized()){
        
        wrote = true;
        for (unsigned int i=0; i<len; i++){
            wrote = serial->writeByte(command.at(i)) ? wrote : false;
        }
        wrote = serial->writeByte(endChar) ? wrote : false;
        
        if (wrote) {
            // report success to console
            ofLogNotice("Commmander") << "sent command: " << command;
        } else {
            // report error to console
            ofLogError("Commander") << "failed to send command: " << command;
        }
    }
    return wrote;
}

Commander::cmdVal Commander::getNext(){
    cmdVal cv;
    if (cmdQueue.size() > 0){
        cv = cmdQueue[0];
        cmdQueue.pop_front(); // destroy oldest in queue
    }
    
    return cv; // empty if none in queue
}

bool Commander::getNext(char* cmd, unsigned long *val){
    bool hasNext = false;
    if (cmdQueue.size() > 0){
        hasNext = true;
        *cmd = cmdQueue[0].cmd;
        *val = cmdQueue[0].val;
        cmdQueue.pop_front(); // destroy oldest in queue
    }
    return hasNext; // false if none in queue (cmd + val unchanged)
}

/* private */


// converts & validates buffer to cmdVal
// -------------------------------------
Commander::cmdVal Commander::cvtBufToCmdVal(){
    
    cmdVal cv;
    
    if (bufLen > 0) { // we have buffer data
        
        // validate cmd style ('A'-'Z') at 1st spot
        
        if (buf[0] >= 65 && buf[0] <= 90) {
            
            // try to convert rest of buffer to ulong
            
            for (int i = 1; i < bufLen; i++) {
                
                // validate val style ('0'-'9')
                
                if (buf[i] >= 48 && buf[i] <=57) {
                    
                    char c = buf[i];
                    cv.val *= 10; // next dec place
                    cv.val += (c - '0'); // add digit
                    
                    if (i == bufLen-1) { // we made it
                        cv.cmd = buf[0]; // save command
                    }
                }
                else break; // end early
            }
        }
    }
    
    if (cv.cmd == 0){
        // report error to console
        ofLogError("Commander") << "cannot convert buffer to cmd/val pair: " << buf;
    }
    
    return cv; // will return cmd == 0 if invalid
}

