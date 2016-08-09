/*
 valid cmds:
 'H' = handshake (input H1, response: H1)
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
 

 error code reporting:
 E0 = invalid buffer (cannot parse to cmd/val pair)
 E1 = buffer overflow (too much data on serial port, no endChar)
 E2 = invalid cmd (unrecognized)
 E3 = invalid val
 E4 = command queue overflow (too many commands without running them)
 E5 = output queue overflow (too many outputs queued without sending)
*/

#define ERR 'E'
#define INVALID_BUFFER 0 
#define BUFFER_OVERFLOW 1
#define INVALID_CMD 2
#define INVALID_VAL 3
#define CMDQUEUE_OVERFLOW 4
#define OUTQUEUE_OVERFLOW 5

const int maxBufLen = 10; // 10 chars: 1 cmd char + 9 digits unsigned long
const int maxQueueLen = 20; // max cmds to store in cmdQueue or outQueue

// global struct for custom cmd/val pair
struct cmdVal {
  char cmd = 0; // null == '\0'
  unsigned long val = 0;
};


class Commander {
  
public:

  Commander() { 
    memset(buf,0,sizeof(buf)); // init buff to 0
  }

  bool serialBegin() { Serial.begin(baudRate); while(!Serial){} }
  bool serialBegin(long baud) { baudRate = baud; serialBegin(); }

  void setEndChar(char ec) { endChar = ec; } //set the char to end a comm/val pair, default is '\n' (newline)
  char getEndChar() { return endChar; } // returns current endChar

  /* INPUT */

  // check serial port for incoming data, parse and add valid cmdVal pairs to cmdQueue until no more data
  void parseAllIncoming();
  
  cmdVal getNextCmdVal();                                       // get next cmdVal pair in cmdQueue - 
                                                                //   returns empty cmdVal (cmd == val == 0) if nothing in cmdQueue
  void getNextCmdVal(char* cmd, unsigned long* val);            // alternative to above, write to pointers
  
  int getNumCmds() { return numCmds; }                          // return number of commands in cmdQueue
  bool haveCmds() { return (numCmds > 0 ? true : false); }      // return true if have cmds in queue
  void flushCmdQueue()                                          // clear the input cmd queue
    { memset(cmdQueue,0,sizeof(cmdQueue)); numCmds = 0; firstCmd = 0; }
  
  /* OUTPUT */
  
  void queueOut(char cmd, unsigned long val);                   // queue a cmdVal pair for output to serial
  void sendOutQueue();                                          // send all cmdVal pairs in output queue
  void sendCmd(char cmd, unsigned long val)                     // immediately send single cmdVal pair to serial
    { Serial.print(cmd); Serial.print(val); Serial.print(endChar); }
    
  int getNumOuts() { return numOuts; }                          // returns number of output cmdVals queued
  void flushOutQueue()                                          // clear the output cmd queue
    { memset(outQueue,0,sizeof(outQueue)); numOuts = 0; }
    

private:

  cmdVal cvtBufferToCmdVal (char * buf, int bufLen);  // returns cmdVal.cmd == 0 if invalid
  
  void addToCmdQueue (char cmd, unsigned long val); // add a cmd val pair to cmdQueue

  cmdVal cmdQueue[maxQueueLen]; // input command + value queue: stores up to 100 pairs
  int numCmds = 0; // tracks number of input cmdVals queued
  int firstCmd = 0; // tracks place in array where first queued cmd is
                    //  queue array wraps around, so a full queue of 10 cmds could be ordered: {5,6,7,8,9,0,1,2,3,4} (firstCmd == 5)

  cmdVal outQueue[maxQueueLen]; // cmdVal output queue
  int numOuts = 0; // tracks number of output cmdVals queued

  long baudRate = 115200; // default to highest baud rate
  char endChar = '\n'; // default to new line char as end val

  char buf[11]; 
  int bufLen = 0;

};

void Commander::parseAllIncoming() {

  while (Serial.available() > 0){
    
    // write to buffer until we reach max buffer length or get an endChar
    
    char c = Serial.read(); // get in byte as char

    if (c == endChar) { // if we've reached an endChar
      
      // try to convert buffer to cmd/val pair

      cmdVal cv = cvtBufferToCmdVal(buf, bufLen);   // convert buffer to cmdVal (char and unsigned long)

      if (cv.cmd != 0){
        if (cv.cmd == 'H' && cv.val == 1) { // handshake, send response
          sendCmd('H',1);
        } else {
          addToCmdQueue(cv.cmd, cv.val);  // add to queue 
        }
      } else {
        sendCmd(ERR,INVALID_BUFFER); // report error on serial
      }
      // clear buffer, start fresh
      memset(buf,0,sizeof(buf));  // init to 0
      bufLen = 0;
      
    }

    else { // not an end char, save char to buffer
    
      if (bufLen == 11) {   // if buffer overflowing
        
        // clear buffer, start fresh
        memset(buf,0,sizeof(buf));  // init to 0
        bufLen = 0;

        sendCmd(ERR,BUFFER_OVERFLOW);   // send error code to serial

        // read serial until we hit an endChar to clear junk
        
        while (Serial.available() > 0 && ((char)Serial.read() != endChar)){}

      } else { // room in buffer, save it
        
        // add to buffer
        buf[bufLen++] = c;  // increment buf len after copy
      }

    }
  }
}

// converts & validates buffer to cmdVal
// ---------------------------------

cmdVal Commander::cvtBufferToCmdVal (char * buf, int bufLen) {

  cmdVal cv;
  
  if (bufLen > 0) { // we have buffer data

    if (buf[0] >= 65 && buf[0] <= 90) {   // valid cmd style ('A'-'Z') at 1st spot

      // try to convert rest of buffer to ulong
      
      for (int i = 1; i < bufLen; i++) {
        
        if (buf[i] >= 48 && buf[i] <=57) {   // valid val style ('0'-'9')
          
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
  
  return cv; // will return cmd == 0 if invalid
}


// returns next cmdVal pair in queue
// ---------------------------------

cmdVal Commander::getNextCmdVal() {
  
  cmdVal cv = cmdQueue[firstCmd]; // store the cmdVal pair
  cmdQueue[firstCmd].cmd = 0; cmdQueue[firstCmd].val = 0; // now clear cmdVal pair

  if (++firstCmd >= maxQueueLen) firstCmd -= maxQueueLen; // increment & wrap around
  if (--numCmds <= 0) flushCmdQueue(); // decrement and reset queue if no cmds left
  
  return cv;
}

void Commander::getNextCmdVal(char* cmd, unsigned long* val){
  cmdVal cv = getNextCmdVal();
  *cmd = cv.cmd;
  *val = cv.val;
}


// queues an output cmdVal
// ---------------------------------

void Commander::queueOut (char cmd, unsigned long val) {

  if (numOuts >= maxQueueLen) { // outQueue overflow
    flushOutQueue();
    sendCmd(ERR,OUTQUEUE_OVERFLOW); // E4 == error code for output queue overflow
  }
  outQueue[numOuts].cmd = cmd;
  outQueue[numOuts].val = val;
  numOuts++;
  
}


// sends all queued output cmdVals to serial, clears queue
// ---------------------------------

void Commander::sendOutQueue() {
  for (int i=0; i<numOuts; i++){
    sendCmd(outQueue[i].cmd,outQueue[i].val);
  }
  flushOutQueue();
}


// adds a cmd val pair to cmdQueue in appropriate spot
// ---------------------------------

void Commander::addToCmdQueue (char cmd, unsigned long val){

  // if cmdQueue is full, clear it and send error code
  
  if (numCmds >= maxQueueLen){
    // clear queue
    flushCmdQueue();
    sendCmd (ERR,CMDQUEUE_OVERFLOW); // E3 == error code for cmdQueue overflow
  }
  
  // add cmd val pair to queue in appropriate spot
  
  int nextSpot = firstCmd + numCmds;
  if (nextSpot >= maxQueueLen) nextSpot -= maxQueueLen; // wrap around to beginning of queue
  cmdQueue[nextSpot].cmd = cmd;
  cmdQueue[nextSpot].val = val;
  numCmds++; // increment num cmds in queue
}


