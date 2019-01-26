/*====================================================================================
  Serial to array to move V8
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


//there is a multitude of if statements because Switch / Case blocks work only with int or char variables, but not strings in Arduino

int totalMotors = 3;
#define BAUDRATE  1000000
#define buttonPin 23 //Opencm 9.04 internal button /*This will not compile if your board is set to anything but OpenCM 9.04*/
#define button2Pin 22 //External Button ------ Used to reset data
#include <DynamixelWorkbench.h>
//Setting Serial Port for Dynamixel Control Board
#if defined(__OPENCM904__)
#define DEVICE_NAME "1" //Dynamixel on Serial3(USART3)  <-OpenCM 485EXP
#elif defined(__OPENCR__)
#define DEVICE_NAME "1"
#endif
const int TotalIDs = totalMotors + 1;
const int vel = 60; //This is the velocity the morors will use (DynamixelWorkbench)
const int acc = 100; //This is the acceleration the motors will use (DynamixelWorkbench)
const int MatrixWidth = TotalIDs;
int widthFinal = MatrixWidth;//Width final can change depedning on the serial data
/*====================================================================================
  Although the width is 1 + the number of motors,
  the Total IDs isn't 0 indexed, but the array is,
  so it conveniently works out that the array width is the same as the MaxIDs number
  ====================================================================================*/
DynamixelWorkbench dxl_wb;
/*Establishing variables and array for serial reads */
int buttonState;
int button2State;
int lengthFinal = 0;
bool hasData = 0;
String input;
int32_t motorPosition;
unsigned long currentTime;
/*====================================================================================
   <dataType>delay, pos for motor ID 1, pos for motor ID 2, pos for motor ID 3, delay2, pos2-1, pos2-2, pos2-3 .../r
   This is the declaration of the empty array which is later filled from Serial
  ====================================================================================*/

void setup() {
  //=========================== Serial initializations ===============================
  Serial.begin(57600);//Serial port Dynamixel
  /*Serial2.write("AT+NAME=SashaMocoBT");
  Serial2.write("AT+BAUD=57600,N");*/
  Serial2.begin(9600); //Serial port for Bluetooth
  //============================ Motor initializations ===============================
  dxl_wb.begin(DEVICE_NAME, BAUDRATE);
  for (int i = 1; i < TotalIDs; i++) {
    dxl_wb.ping(i);
    //dxl_wb.jointMode(i, vel, acc);
    dxl_wb.jointMode(i);
    //============================= Debugging Readout ================================
    Serial2.print("Motor ");
    Serial2.print(i);
    Serial2.print(" initialized");
    Serial2.println();
  }
}



void loop() {
  String nuveau = Serial2.readStringUntil('/r');
  if (nuveau != input and nuveau != "" and nuveau != "^M" and hasData == 0) {
    if (nuveau.startsWith("<moveData>")) {
      input = nuveau;
      hasData = 1;
      Serial2.print("String recieved, press button to move");
      Serial2.print(input);
      Serial2.println();
      Serial2.flush();
      nuveau = "";
    }
  }

  pinMode(buttonPin, INPUT_PULLDOWN);
  buttonState = digitalRead(buttonPin);
  if (buttonState == HIGH) {
    motorMove(input);
    Serial2.flush();
    nuveau = "";
  }
  pinMode(button2Pin, INPUT_PULLDOWN);
  button2State = digitalRead(button2Pin);
  if (button2State == HIGH) {
    Serial2.println("Move data cleared");
    hasData = 0;
    Serial2.flush();
    nuveau = "";
  }
  if (nuveau.startsWith("<newData>")) {
    hasData = 0;
    Serial2.println("Ready for data read");
    input = "";
    Serial2.flush();
    nuveau = "";
  }
  if (input == "") {
    hasData = 0;
  }
  if (nuveau.startsWith("<moveTrigger>")) {
  if (nuveau == "<moveTrigger>" ) {
      if (hasData == 1) {
        motorMove(input);
      }
      else {
        Serial2.println("Error, no data");
      }
      Serial2.flush();
      nuveau = "";
    }
    else {
      Serial2.println("ERROR: <moveTrigger> has no additional data");
    }
  }
}

void motorMove(String input) {

  //String parse begin

  Serial2.println("Data recieved");

  input.trim(); //removes spaces from string

  int lengthInput = 0;//Establish the lengthInput variable

  int lengthString = input.length();//Find the number of characters in the recieved string

  input = typeRemover(input, lengthString);

  lengthString = input.length();

  Serial2.println("String Processed " + input);

  if (input.startsWith("0") or input.startsWith("1") or input.startsWith("2") or input.startsWith("3") or input.startsWith("4") or input.startsWith("5") or input.startsWith("6") or input.startsWith("7") or input.startsWith("8") or input.startsWith("9")) {
    Serial2.println("String Correct.");
  }
  else {
    Serial2.println("ERROR: Input does not start with an integer. Check Packet.");
    return;
  }


  Serial2.println("Move begin");

  for (int i = 0; i < lengthString; i++) {
    if (input.substring(i, i + 1) == ",") {
      lengthInput++;
    }//For the length of the string, incrrease the input length by 1 for every comma (input length is the number of rows in the array)
  }

  Serial2.println("Length input is ");
  Serial2.println(lengthInput);

  char inputArray[lengthString];
  input.toCharArray(inputArray, lengthString);//Converts the serial data into a 1d char array

  Serial2.println("Char array built");

  String numbers[lengthInput];//Create a 1d int array of strings



  int k = 0;

  for (int i = 0; i < lengthString; i++) {
    if (input.substring(i, i + 1) == ",") {
      k++;
    } else {
      numbers[k] = numbers[k] + inputArray[i];
    }
  }//fill the string array

  Serial2.println("String array built");

  int integers[lengthInput];
  for (int i = 0; i < lengthInput; i++) {
    integers[i] = numbers[i].toInt();
  }//Create and fill a 1d array of integers

  Serial2.println("Integer Conversion Complete");

  int widthFinal = MatrixWidth;
  int lengthFinal = lengthInput / widthFinal;

  int MoveMatrix[lengthFinal][widthFinal];

  Serial2.print("MoveMatrix Array Re-Declared: ");
  Serial2.print("Length of array is ");
  Serial2.print(lengthFinal);
  Serial2.print(" Width of array is ");
  Serial2.print(widthFinal);
  Serial2.println();


  k = 0;

  for (int i = 0; i < lengthFinal; i++) {
    for (int j = 0; j < widthFinal; j++) {
      MoveMatrix[i][j] = integers[k];
      k++;
      Serial2.println(MoveMatrix[i][j]);
    }//Fill move matrix
  }
  for (int i = 0; i < lengthFinal; i++) {
    for (int j = 0; j < widthFinal; j++) {
      Serial2.print(MoveMatrix[i][j]);
      Serial2.print(" , ");
    }
    Serial2.println();
  }//Read the array back to Serial2
  hasData = 1; //Indicate that there has been data recieved
  Serial2.println("Data processed, ready to move");

  {
    for (int i = 0; i < lengthFinal ; i++) {
      if (MoveMatrix[i][0] == 0) {
        continue; //Continues the loop if the duration is 0
      }
      if (MoveMatrix[i][0] == -1) {
        break; //Ends when the duration is set to -1
      }

      else {
        for (int j = 1; j < widthFinal; j++) {

          currentTime = millis();

          int previousPosition;
          int deltaPosition;
          int currentPosition;
          int deltaTime;

          if (i != 0) {
            previousPosition = MoveMatrix[i - 1][j];
          }
          else {
            previousPosition = dxl_wb.itemRead(j, "Present_Position");
          }

          currentPosition = MoveMatrix[i][j];

          if (currentPosition > previousPosition) {
            deltaPosition = currentPosition - previousPosition;
          }
          if (currentPosition <= previousPosition) {
            deltaPosition = previousPosition - currentPosition;
          }

          deltaTime = MoveMatrix[i][0];

          int goalVel = deltaPosition / deltaTime; //This is in units of 360/4096 degrees per millisecond

          const int goalTargetRatio = 234375 / 3664;  // 360/4096 d/ms = 1875/128 rpm; 234375/3664 0.229 RPMs per d/ms

          Serial2.print("goalVel = ");
          Serial2.println(goalVel);
          Serial2.print("GTR = ");
          Serial2.println(goalTargetRatio);

          int targetVel = goalVel * goalTargetRatio; //This is the velocity in units of 0.229 [rev/min] as per the library spec

          //TO ADD: Scan to next velocity to calculate Profile Acceleration

          Serial2.print("Goal Velocity (0.229RPM) is ");
          Serial2.println(targetVel);

          dxl_wb.itemWrite(j, "Profile_Velocity", targetVel);
          dxl_wb.goalPosition(j, MoveMatrix[i][j]); //Moves motor to corresponding position
          Serial2.print("Coords are ");
          Serial2.println(MoveMatrix[i][j]);
          Serial2.print("ID is ");
          Serial2.println(j);
          Serial2.print("Read positon of motor is ");
          motorPosition = dxl_wb.itemRead(j, "Present_Position");
          Serial2.println(motorPosition);
        }
        Serial2.println("Time " + MoveMatrix[i][0]);
        delay(MoveMatrix[i][0]);
        /*int deltaTime = MoveMatrix[i][0];
          if(millis() == currentTime + deltaTime){
          continue;
          }
          else{
          delayMicroseconds(1);
          }*/
      }
    }
  }
  Serial2.println("Move end");
  return;
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
      input = input + ",";                                    //Change to doesn't end with above and uncomment to add a comma
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

  if (input.startsWith(">")) {
    input.remove(0, 1);
  }

  return input;
}

