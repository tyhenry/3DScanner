#include <CheapStepper.h>

CheapStepper stepper = CheapStepper();

void setup() {
  stepper.setRpm(12);
  stepper.newMove(true, 256);
}

void loop() {

  if (stepper.getStepsLeft() == 0) stepper.newMove(true, 256);
  stepper.run();

}
