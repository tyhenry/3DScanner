// valid cmds: 
// 'R' = set RPM
// 'G' = set gear ratio (x 100)
// 'M' = move one turn
// 'P' = take picture
// 'T' = rotate turntable (continuous)
// 'C' = set turns per circle
// 'K' = set clockwise (if val == 1, else counter-clockwise)
// 'A' = run autoscan (full circle with turnsPerCircle picture)
// 'W' = set wait time between pictures (ms)
// 'F' = force camera ready
// 'S' = stop moving (& cancel autoscan)

// error code reporting:
// 'E0' = invalid serial byte (i.e. not 'A'-'Z' or '0'-'9' or '\0'
// 'E1' = cmd/val mismatch (got one, but not the other)
// 'E2' = invalid val sequence stored somehow? (i.e. not '0'-'9')
// 'E3' = queue overflow (queue is cleared in this case)
// 'E4' = invalid cmd in queue

// arduino will report updated values once commands are run for confirmation


// struct for custom cmd/val pair (for serial connection)
struct cmdVal {
  char cmd = 0; // null
  unsigned long val = 0;
};

cmdVal cmdQueue[100]; // command + value queue: stores up to 100 pairs if backlog

void runCommands() {

  for (int i=0; i<100; i++){ // loop through queue
    if (cmdQueue[i].cmd == 0){
      // done, reset queue and break
      memset(cmdQueue,0,sizeof(cmdQueue)); // reset queue array to all null
      break;
    }
    else {
      char cmd = cmdQueue[i].cmd;
      unsigned long val = cmdQueue[i].val;
      // run command

      if (cmd == 'R'){
        stepper.setRpm(val);
        // report back
        Serial.print('R'); Serial.print(stepper.getRpm()); Serial.print('\0');
      }
      else if (cmd == 'G'){
        gearRatioX100 = val;
        // report back
        Serial.print('G'); Serial.print(gearRatioX100); Serial.print('\0');
      }
      else if (cmd == 'M'){
        stepper.newMove(clockwise, stepsPerTurn);
        // report back
        Serial.print("M0");
      }
      else if (cmd == 'P'){
        stepper.stop();
        triggerCamera();
        bCameraReady = false;
        // report back
        Serial.print("P0");
      }
      else if (cmd == 'T'){
        stepper.move(clockwise, (stepsPerTurn/4)); // quarter-turn
        bCameraReady = true;
        // report back
        Serial.print("T0");
      }
      else if (cmd == 'C'){
        turnsPerCircle = val;
        calcStepsPerTurn();
        // report back
        Serial.print('C'); Serial.print(stepsPerTurn); Serial.print('\0');
      }
      else if (cmd == 'K'){
        if (val == 1){
          clockwise = true;
        } else {
          clockwise = false;
        }
        // report back
        Serial.print('K'); Serial.print(clockwise ? '1':'0'); Serial.print('\0');
      }
      else if (cmd == 'A'){
        runAutoscan();
        Serial.print("A0");
      }
      else if (cmd == 'W'){
        waitBetweenMoves = val;
        Serial.print('W'); Serial.print(val); Serial.print('\0');
      }
      else if (cmd == 'F'){
        bCameraReady = true;
        Serial.print('F'); Serial.print(bCameraReady ? '1':'0'); Serial.print('\0');
      }
      else if (cmd == 'S'){
        stepper.stop();
        autoScanMovesLeft = 0;
        Serial.print('S'); Serial.print(autoScanMovesLeft); Serial.print('\0');
      }
      else {
        Serial.print("E4"); // send error code - invalid cmd in queue
      }
    }
  }
  
}


// serialEvent() runs between loop() iterations, whenever serial data is available

void serialEvent() {

  // receive command/value pair

  // string cmd/val style: "R12" (inlcudes '\0' null char at end)
  // e.g. uppercase letter followed by number sequence, ended with null character

  bool bCmd = false; // did we get a command (uppercase letter)?
  bool bVal = false; // did we get a value (number sequence)?
  bool bValEnd = false; // did we get a null terminator to end cmd/val?
  
  char cmd; // this will save the command
  char val[50]; // this will save the number sequence (value)
  unsigned int valN = 0; // saves our place in the val array
  memset(val,0,sizeof(val)); // init val array to all null
  
  while (Serial.available()) { // new data on serial port
    
    // get the next byte:
    char inChar = (char)Serial.read();

    // is it a cmd char?
    if (inChar >= 41 && inChar <= 90) { // 'A'-'Z'
       
       cmd = inChar;
       bCmd = true;
       memset(val,0,sizeof(val)); // reset val array to all null
       valN = 0;
    }
    // is it a val sequence?
    else if (inChar >= 48 && inChar <= 57 && bCmd) { // '0'-'9' & we have a cmd already

      val[valN] = inChar;
      valN++;
      bVal = true;
      
    }
    // is it an end value?
    else if (inChar =='\0'){ 

      if (bCmd && bVal) { // we got both a cmd char and num sequence

        // add cmdVal to cmdQueue

        // find the next empty spot in the queue
        int nextPlace = 0;
        for (int i=0; i<100; i++){
          if (i == 99){
            // queue full, clear it and set place to 0
            memset(cmdQueue,0,sizeof(cmdQueue)); // reset queue array to all null
            nextPlace = 0;
            break;
            Serial.print("E3"); // send queue full error code
          }
          if (cmdQueue[i].cmd = 0){
            // we have an empty spot in the queue
            nextPlace = i;
            break;
          }
        }

        // place cmdVal in queue

        // convert val array to unsigned long
        unsigned long valLong = 0;
        bool goodVal = true;
        for (int i = 0; i < 50; i++) {
           char c = val[i];
           if ((c < '0' || c > '9') && c != 0){ // bad data
            goodVal = false;
            break;
           }
           else if (c == 0){ // end of sequence
            if (i == 0){ // no data
              goodVal = false;
            }
            break;
           }
           valLong *= 10;
           valLong += (c - '0');
        }
        if (goodVal){
          // valid data, place in queue
          cmdQueue[nextPlace].cmd = cmd;
          cmdQueue[nextPlace].val = valLong; 
        }
        else {
          // invalid val sequence
          Serial.print("E2"); // send invalid val sequence error code
        }
        
      }
      else { // we don't have both a cmd and val, reset everything
        bCmd = bVal = bValEnd = false;
        valN = 0;
        memset(val,0,sizeof(val)); // reset val array to all null
        Serial.print("E1"); // send cmd val mismatch error code to serial
      }

    }
    // invalid serial byte, reset everything
    else { 
      bCmd = bVal = bValEnd = false;
      valN = 0;
      memset(val,0,sizeof(val)); // reset val array to all null
      Serial.print("E0"); // send invalid serial byte error code to serial
    }
  }
}

