#3D Scanner

Code for custom-build 3D scanner:

- Arduino based turntable with photo trigger  
  - uses custom serial commander IO class
  - uses custom CheapStepper 28BYJ-48 stepper motor library
- openFrameworks GUI
  - in progress: control Arduino through serial commands, update GUI
  - to do: watch folder with auto photo ingest + display  
    - uses Canon EOS Utility to auto-ingest photos over USB


##Arduino
####**/scanner_commander**  

- scanner_commander.ino:  
  arduino sketch for serial controlled turntable/photo trigger
- Commander.h: serial control parsing and queuing
- Scanner.h: runs turntable and photo trigger
  - support for custom motor:turntable gearing
  - autoscanning mode (run full rotation of photos and moves)
  - uses CheapStepper 28BYJ-48 stepper motor controller library
  
  

####*more documentation coming soon*
