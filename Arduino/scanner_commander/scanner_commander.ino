#include "Scanner.h"
#include "Commander.h"

Commander commander; // serial cmd/val IO
Scanner scanner; // scanner control

void setup() {
  
  commander.serialBegin(115200);
  scanner.setIrLedPin(12);
  scanner.setCamLedPin(13); // onboard led
  scanner.setTurntableRotationSteps(16384); // motor:turntable gearing 1:4
}

void loop() {

  if (scanner.update()){
    sendUpdate();
  }

  // run all commands in commander's cmd queue
  while (commander.haveCmds()) {
    char cmd; unsigned long val;
    commander.getNextCmdVal(&cmd,&val); // grabs from & empties queue
    runCommand(cmd,val);
  }

  // get all new commands from serial, move to queue
  commander.parseAllIncoming();
  
}

void sendUpdate(){
  commander.sendCmd('A', scanner.getAutoscanMovesLeft());
  commander.sendCmd('S', scanner.getStepperPos());
  commander.sendCmd('M', (scanner.isMoving() ? 1:0));
  commander.sendCmd('P', (scanner.isShooting() ? 1:0));
}

void runCommand(char cmd, unsigned long val) {

  // parse, run command, send report

  if (cmd == 'R'){ // set motor RPM
    if (val > 0) scanner.setMotorRpm(val); // set
    commander.sendCmd('R', scanner.getMotorRpm()); // report
  }
  else if (cmd == 'M'){ // turn
    if (val > 0) scanner.turn();
    commander.sendCmd('M', (scanner.isMoving() ? 1:0)); // report
  }
  else if (cmd == 'P'){ // take picture
    if (val > 0) scanner.takePhoto();
    commander.sendCmd('P', (scanner.isShooting() ? 1:0)); // report
  }
  else if (cmd == 'T'){ // rotate turntable
    scanner.rotateTurntable();
    commander.sendCmd('S', scanner.getStepperPos()); // report current step
  }
  else if (cmd == 'C'){ // set # turns per circle
    if (val > 0) scanner.setTurnsPerCircle(val); // set
    commander.sendCmd('C', scanner.getTurnsPerCircle()); // report
  }
  else if (cmd == 'K'){ // set clockwise/ccw
    if (val == 1 || val == 0) scanner.setClockwise(val); // 1 for cw, 0 for ccw
    commander.sendCmd('K', scanner.getClockwise()); // report
  }
  else if (cmd == 'A'){ // start autoscan
    if (val == 1) scanner.startAutoscan(); // 1 for start
    else if (val == 0) {
      scanner.stopAutoscan(); // 0 for stop
      sendUpdate();
    }
    commander.sendCmd('A', scanner.getAutoscanMovesLeft()); // report # moves left in autoscan
  }
  else if (cmd == 'W'){ // set wait (ms) after photo before next move
    if (val > 0) scanner.setWaitAfterPhoto(val);
    commander.sendCmd('W', scanner.getWaitAfterPhoto()); // report
  }
  else if (cmd == 'G'){ // set # steps in turntable rotation
    if (val> 0) scanner.setTurntableRotationSteps(val);
    commander.sendCmd('G', scanner.getTurntableRotationSteps());
  }
  else if (cmd == 'S'){ // move stepper to step pos
    commander.sendCmd('M', 1); // moving
    scanner.moveToStep(val);
    commander.sendCmd('M', 0); // stopped
    commander.sendCmd('S', scanner.getStepperPos());
  }
  else if (cmd == 'I'){ // return stepper step pos
    commander.sendCmd('S', scanner.getStepperPos());
  }
  else if (cmd == 'Q'){ // return number cmds in queue or flush queue
    if (val > 0) commander.flushCmdQueue(); // flush
    commander.sendCmd('Q', commander.getNumCmds()); // report #
  }
  else {
    commander.sendCmd(ERR,INVALID_CMD); // send invalid command error code (E4)
  }
  
}

