/*====================================================================================
  MotionKeyframer-V0.3
  ====================================================================================

  This is a combination of Serial-To-Array-To-Move V10 and MoveRecorder V1

  ====================================================================================

  The purpose of this code is to take an input of position and time data in the style
  of animation keyframes from Serial, in this case a bluetooth LE adapter paired to a
  phone running a control application, and use this data to drive motors.

  ====================================================================================
  This code will take a string formatted as:
        time, pos1, pos2, pos3, time2, pos1,2 pos 2,2 pos3,2,.../r
  Wherein time is followed by a number of positions equal to the number of motors
  and the number of motors is set correctly on both the sender and reciever
  and the string is formatted as above, and terminated with a carriage return
  And it will move these motors according to these "Keyframes"
  This is coded to run on an OpenCM 9.04 from Robotis, using Protocol 2.0 XL series
  Smart Servos, however it can be easily adapted to any other type of servo motor.
  ======================================================================================*/

int totalMotors = 3;
#define BAUDRATE  1000000
#define buttonPin 23 //Opencm 9.04 internal button /*This will not compile if your board is set to anything but OpenCM 9.04*/
#define button2Pin 22 //External Button ------ Used to reset data
#define recordButton 21

/*====================================================================================
  Although the width is 1 + the number of motors,
  the Total IDs isn't 0 indexed, but the array is,
  so it conveniently works out that the array width is the same as the MaxIDs number
  ====================================================================================*/

#include <DynamixelWorkbench.h>
//Setting Serial Port for Dynamixel Control Board
#if defined(__OPENCM904__)
#define DEVICE_NAME "1" //Dynamixel on Serial3(USART3)  <-OpenCM 485EXP
#elif defined(__OPENCR__)
#define DEVICE_NAME "1"
#endif
const int TotalIDs = totalMotors + 1;
const int vel = 60; //Placeholder Velocity
const int acc = 100; //Placeholder Acceleration
const int MatrixWidth = TotalIDs;
int widthFinal = MatrixWidth;//Width final can change depedning on the serial data
int buttonState;
int button2State;
int lengthFinal;
bool hasData;
String input;
int32_t motorPosition;
unsigned long currentTime;
int recordingButton;


DynamixelWorkbench dxl_wb;
/*Establishing variables and array for serial reads */
/*====================================================================================
   <dataType>delay, pos for motor ID 1, pos for motor ID 2, pos for motor ID 3, delay2, pos2-1, pos2-2, pos2-3 .../r
   This is the declaration of the empty array which is later filled from Serial
  ====================================================================================*/

void setup() {
  pinMode(button2Pin, INPUT_PULLDOWN);
  pinMode(buttonPin, INPUT_PULLDOWN);

  //=========================== Serial initializations ===============================
  Serial.begin(57600);//Serial port Dynamixel
  /*Serial2.begin(38400);
    Serial2.write("AT+NAME=SashaMocoBT");
    Serial2.write("AT+BAUD=57600,N");
    Serial2.begin(57600);*/
  Serial2.begin(9600); //Serial port for Bluetooth
  //============================ Motor initializations ===============================
  dxl_wb.begin(DEVICE_NAME, BAUDRATE);
  for (int i = 1; i < TotalIDs; i++) {
    dxl_wb.ping(i);
    //dxl_wb.jointMode(i, vel, acc);
    dxl_wb.jointMode(i);
    dxl_wb.itemWrite(i, "Torque", 0);
    //============================= Debugging Readout ================================
    Serial2.print("Motor ");
    Serial2.print(i);
    Serial2.print(" initialized");
    Serial2.println();
  }
}



void loop() {
  int dataType = 0;
  String nuveau = Serial2.readStringUntil('/r');
  buttonState = digitalRead(buttonPin);
  button2State = digitalRead(button2Pin);
  recordingButton = digitalRead(recordButton);

  if (input == "") {
    hasData = 0;
  }

  if (nuveau.startsWith("<readMove>")) {
    dataType = 1;
  }
  
  if (recordingButton == HIGH) {
    dataType = 1;
  }

  if (nuveau.startsWith("<moveData>")) {
    dataType = 2;
  }

  if (nuveau.startsWith("<newData>")) {
    dataType = 3;
  }

  if (button2State == LOW) {
    dataType = 3;
  }

  if (nuveau.startsWith("<moveTrigger>")) {
    dataType = 4;
  }


  if (buttonState == HIGH) {
    dataType = 4;
    /*nuveau = "<readMove>24,240";
      dataType = 1;
    */
  }




  switch (dataType)
  {
    case 0 : {
        break;
      }
    case 1 :
      {
        Serial2.println("Start Recording");
        input = moveRead(nuveau);
        hasData = 1;
        nuveau = "";
        Serial2.flush();
        dataType = 0;
        break;
      }
    case 2 :
      {
        input = nuveau;
        hasData = 1;
        Serial2.print("String recieved, press button to move");
        Serial2.print(input);
        Serial2.println();
        Serial2.flush();
        nuveau = "";
        dataType = 0;
        break;
      }
    case 3 :
      {
        hasData = 0;
        Serial2.println("Ready for data read");
        input = "";
        Serial2.flush();
        nuveau = "";
        dataType = 0;
        break;
      }
    case 4 :
      {
        if (hasData == 1) {
          motorMove(input);
          dataType = 0;
        }
        else {
          Serial2.println("ERROR: no data");
          dataType = 0;
        }
        break;
      }
    default:
      {
        Serial2.println("ERROR: unknown data type");
        dataType = 0;
        break;
      }
  }
}

void motorMove(String input) {

  for (int i = 1; i < TotalIDs; i++) {
    dxl_wb.itemWrite(i, "Profile_Velocity", 0);
    dxl_wb.itemWrite(i, "Torque", 1);
    dxl_wb.goalPosition(i, 0);
  }

  //String parse begin

  bool hasMoved = 0;

  //Serial2.println("Starting MovePrep");

  input.trim(); //removes spaces from string

  int lengthInput = 0;//Establish the lengthInput variable

  int lengthString = input.length();//Find the number of characters in the recieved string

  input = typeRemover(input, lengthString);

  lengthString = input.length();

  if (input.startsWith("0") or input.startsWith("1") or input.startsWith("2") or input.startsWith("3") or input.startsWith("4") or input.startsWith("5") or input.startsWith("6") or input.startsWith("7") or input.startsWith("8") or input.startsWith("9")) {
    Serial2.println("String Correct." + input);
  }
  else {
    Serial2.println("ERROR: Input does not start with an integer. Check Packet." + input);
    return;
  }

  for (int i = 0; i < lengthString; i++) {
    if (input.substring(i, i + 1) == ",") {
      lengthInput++;
    }//For the length of the string, incrrease the input length by 1 for every comma (input length is the number of rows in the array)
  }

  char inputArray[lengthString];
  input.toCharArray(inputArray, lengthString);//Converts the serial data into a 1d char array


  String numbers[lengthInput];//Create a 1d int array of strings

  int k = 0;

  for (int i = 0; i < lengthString; i++) {
    if (input.substring(i, i + 1) == ",") {
      k++;
    } else {
      numbers[k] = numbers[k] + inputArray[i];
    }
  }//fill the string array


  int integers[lengthInput];
  for (int i = 0; i < lengthInput; i++) {
    integers[i] = numbers[i].toInt();
  }//Create and fill a 1d array of integers

  int widthFinal = MatrixWidth;
  int lengthFinal = lengthInput / widthFinal;

  int moveMatrix[lengthFinal][widthFinal];


  k = 0;

  for (int i = 0; i < lengthFinal; i++) {
    for (int j = 0; j < widthFinal; j++) {
      moveMatrix[i][j] = integers[k];
      k++;
      //Serial2.println(moveMatrix[i][j]);
    }//Fill move matrix
  }
  /*for (int i = 0; i < lengthFinal; i++) {
    for (int j = 0; j < widthFinal; j++) {
      Serial2.print(moveMatrix[i][j]);
      Serial2.print(" , ");
    }
    Serial2.println();*/
  //Read the array back to Serial2
  hasData = 1; //Indicate that there has been data recieved
  //Serial2.println("Data processed, ready to move");

  {
    //startMove:
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

          const double goalTargetRatio = 14.6484375;  // 360/4096 d/ms = 1875/128 rpm; 234375/3664 0.229 RPMs per d/ms

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
}

String typeRemover(String input, int lengthString) {
  if (input.endsWith("/r")) {
    input.remove(lengthString - 2, lengthString); //Removes the carriage return (end marker) from the string, if it is present
  }

  if (input.endsWith("/")) {
    input.remove(lengthString - 1, lengthString);              //Removes the carriage return (end marker) from the string, if it is present
  }

  if (!input.endsWith(",")) {
    if (input != "") {
      input = input + ",";
    }
  }

  if (input.startsWith("<moveData>")) {
    input.remove(0, 9);
  }

  if (input.startsWith("<newData>")) {
    input.remove(0, 8);
  }

  if (input.startsWith("<moveTrigger>")) {
    input.remove(0, 12);
  }

  if (input.startsWith("<readMove>")) {
    input.remove(0, 9);
  }

  if (input.startsWith(">")) {
    input.remove(0, 1);
  }

  return input;
}

String moveRead(String input) {

  for (int i = 1; i < TotalIDs; i++) {
    dxl_wb.itemWrite(i, "Torque", 0);
  }

  Serial2.println("Begin Move Read Process");

  input.trim(); //removes spaces from string

  int lengthInput;//Establish the lengthInput variable

  int lengthString = input.length();//Find the number of characters in the recieved string

  input = typeRemover(input, lengthString);



  if (input.startsWith("0") or input.startsWith("1") or input.startsWith("2") or input.startsWith("3") or input.startsWith("4") or input.startsWith("5") or input.startsWith("6") or input.startsWith("7") or input.startsWith("8") or input.startsWith("9")) {
    Serial2.println("String Correct: " + input);
  }
  else {
    Serial2.println("ERROR: Input does not start with an integer. Check Packet.");
    return input;
  }

  lengthString = input.length();

  Serial2.println(lengthString);

  lengthInput = 2;

  Serial2.print("Length input is ");
  Serial2.println(lengthInput);

  char inputArray[lengthString];
  input.toCharArray(inputArray, lengthString);//Converts the serial data into a 1d char array

  //Serial2.println("Char array built");

  String numbers[lengthInput];//Create a 1d int array of strings

  int k = 0;

  for (int i = 0; i < lengthString; i++) {
    if (input.substring(i, i + 1) == ",") {
      k++;
    } else {
      numbers[k] = numbers[k] + inputArray[i];
    }
  }//fill the string array

  //Serial2.println("String array built");

  int integers[lengthInput];
  for (int i = 0; i < lengthInput; i++) {
    integers[i] = numbers[i].toInt();
  }//Create and fill a 1d array of integers

  int sampleFPS = integers[0];
  int sampleFrames = integers[1];

  Serial2.print("Integer Conversion Complete FPS and FRAMES = ");
  Serial2.print(sampleFPS);
  Serial2.print("FPS ");
  Serial2.print(sampleFrames);
  Serial2.print("Frames");
  Serial2.println();

  double unroundedSampleMSPF = 1000 / sampleFPS;
  double unroundedSampleTimeMs = sampleFrames * unroundedSampleMSPF;
  int sampleMSPF = round(unroundedSampleMSPF);
  int sampleTimeMs = round(unroundedSampleTimeMs);
  String moveData;
  int startTime = millis();
  int previousTime = startTime;
  bool recordingNow = 1;

  Serial2.println(sampleMSPF);
  Serial2.println(sampleTimeMs);
  Serial2.println(recordingNow);
  Serial2.println(startTime);

  //switch (recordingNow) {
   
    //case 0 : {
    if (recordingNow == 0){
        int prevTime = millis();
        Serial2.println("End Rec in 15ms");
        if (millis() >= prevTime + 15 and recordingNow == 0) {
          String moveRead = moveData;
          Serial2.println(moveRead);
          return moveRead;
          //break;
        }
      }
    //case 1 :
     while (recordingNow == 1) {
      {
        if (millis() == previousTime + sampleMSPF) {
          previousTime = millis();
          moveData = moveData + sampleMSPF + ",";
          int motorPosition[totalMotors];
          for (int i = 1; i < TotalIDs; i++) {
            motorPosition[i] = dxl_wb.itemRead(i, "Present_Position");
          }
          for (int i = 1; i < TotalIDs; i++) {
            moveData = moveData + motorPosition[i] + ",";
          }
          Serial2.println(moveData);
        }
        //break;
        //delayMicroseconds(1);
      }
    /*default:
      {
        recordingNow = 1;
        Serial2.println("Default case in recordingNow");
        break;
      }*/
  }


  if (millis() == startTime + sampleTimeMs) {
    recordingNow = 0;
  }


  pinMode(recordButton, INPUT_PULLDOWN);
  int reocrdingButton = digitalRead(buttonPin);
  if (recordingButton == HIGH) {
    recordingNow = 0;
  }
}
