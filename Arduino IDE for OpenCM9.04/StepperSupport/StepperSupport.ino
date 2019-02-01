
void stepperMover(String input) {
  input = stringCleaner(input);

  int moveMatrix[][numberSteppers];

  int_32 goalPosition = moveMatrix[i][j];
  
  volatile int_32 stepperPosition = digitalRead(18);
  
  while (stepperPosition != goalPosition) {
    volatile int_32 stepperPosition = digitalRead(18);

    if (stepperPosition < goalPosition) {
      stepper.steps(1);
    }

    if (stepperPosition > goalPosition) {
      stepper.steps(-1);
    }
  }
}


for (int i = 0; i < lengthFinal ; i++) {

      currentTime = millis();

      if (moveMatrix[i][0] == 0) {
        continue; //Continues the loop if the duration is 0
      }
      if (moveMatrix[i][0] == -1) {
        break; //Ends when the duration is set to -1
      }

      else {
        for (int j = 1; j < widthFinal; j++) {


          float previousPosition;
          float currentPosition;
          float deltaPosition;
          int deltaTime;


          if (i == 0) {
            previousPosition = moveMatrix[i][j];
            currentPosition = moveMatrix[i + 1][j];
          } else {
            previousPosition = moveMatrix[i - 1][j];
            currentPosition = moveMatrix[i][j];
          }

          if (currentPosition > previousPosition) {
            deltaPosition = currentPosition - previousPosition;
          }
          if (currentPosition <= previousPosition) {
            deltaPosition = previousPosition - currentPosition;
          }

          deltaTime = moveMatrix[i][0];

          double unroundedGoalVel = deltaPosition / deltaTime; //This is in units of 360/4096 degrees per millisecond

          int goalVel = round(unroundedGoalVel);

          const double goalTargetRatio = 360/stepsPerRotation;  // 360/4096 d/ms = 1875/128 rpm; 234375/3664 0.229 RPMs per d/ms

          /*Serial2.print("goalVel = ");
            Serial2.println(goalVel);
            Serial2.print("GTR = ");
            Serial2.println(goalTargetRatio);*/

          double unroundedTargetVel = unroundedGoalVel * goalTargetRatio; //This is the velocity in units of 0.229 [rev/min] as per the library spec

          /*Serial2.print("X1 = "); Serial2.println(previousPosition);
          Serial2.print("X2 = "); Serial2.println(currentPosition);
          Serial2.print("dX = "); Serial2.println(deltaPosition);
          Serial2.print("dT = "); Serial2.println(deltaTime);
          Serial2.print("v = "); Serial2.println(unroundedGoalVel);*/


          int targetVel = round(unroundedTargetVel);

          if (targetVel <= 0) {
            targetVel = 1;
          }

          //TO ADD: Scan to next velocity to calculate Profile Acceleration

          dxl_wb.itemWrite(j, "Profile_Velocity", targetVel);
          //while (dxl_wb.itemRead(j, "Present_Position") != moveMatrix[i][j]) {
          dxl_wb.goalPosition(j, moveMatrix[i][j]); //Moves motor to corresponding position
          /*Serial2.print("goal position for ID ");
          Serial2.print(j);
          Serial2.print(" is ");
          Serial2.print(moveMatrix[i][j]);
          Serial2.print(" and velocity is ");
          Serial2.print(targetVel);
          Serial2.println();
          Serial2.print("Coords are ");
            Serial2.println(moveMatrix[i][j]);
            Serial2.print("ID is ");
            Serial2.println(j);
            Serial2.print("Read positon of motor is ");
            motorPosition = dxl_wb.itemRead(j, "Present_Position");
            Serial2.print(motorPosition);*/
          //}

        }
        //Serial2.println("Time " + moveMatrix[i][0]);
        //delay(moveMatrix[i][0]);
        int deltaTime = moveMatrix[i][0];

        if (millis() == currentTime + deltaTime) {
          continue;
        }
        else {
          delayMicroseconds(1);
        }
        //if (i == lengthFinal) {
        //goto finishMove;

      }
    }
  }
finishMove:
  Serial2.println("Move end");
  //if (hasMoved == 1) {
  return;
  /*}
    else {
    goto startMove;
    }*/
