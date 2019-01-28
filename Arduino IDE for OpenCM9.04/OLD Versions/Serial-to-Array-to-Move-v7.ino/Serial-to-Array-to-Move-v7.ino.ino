/*====================================================================================
  Serial to array to move V7
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

const int TotalMotors = 3;
#define BAUDRATE  1000000
#define buttonPin 23 //Opencm 9.04 internal button /*This will not compile if your board is set to anything but OpenCM 9.04*/
#include <DynamixelWorkbench.h>
//Setting Serial Port for Dynamixel Control Board
#if defined(__OPENCM904__)
#define DEVICE_NAME "1" //Dynamixel on Serial3(USART3)  <-OpenCM 485EXP
#elif defined(__OPENCR__)
#define DEVICE_NAME "1"
#endif
const int TotalIDs = TotalMotors + 1;
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

int buttonState = 0;
/*====================================================================================
   {delay, pos for motor ID 1, pos for motor ID 2 + n, pos for motor ID 3 ...},
   This is the declaration of the empty array which is later filled from Serial
  ====================================================================================*/

int lengthFinal = 0;
bool HasData = 0;
String overrideConfirm = "";
String input;


void setup() {
  //=========================== Serial initializations ===============================
  Serial.begin(57600);//Serial port Dynamixel
  Serial2.begin(9600);//Serial port for Bluetooth
  String input = "";
  //============================ Motor initializations ===============================
  dxl_wb.begin(DEVICE_NAME, BAUDRATE);
  for (int i = 1; i < TotalIDs; i++) {
    dxl_wb.ping(i);
    dxl_wb.jointMode(i, vel, acc);
    //============================= Debugging Readout ================================
    Serial2.print("Motor ");
    Serial2.print(i);
    Serial2.print(" initialized");
    Serial2.println();
  }
}



void loop() {

  String nuveau = Serial2.readStringUntil('/r');
  if (nuveau != input and nuveau != "" and nuveau != "^M" and HasData == 0) {
    input = nuveau;
    HasData = 1;
    Serial2.print("String recieved, press button to move");
    Serial2.print(input);
    Serial2.println();
  }
  pinMode(buttonPin, INPUT_PULLDOWN);
  buttonState = digitalRead(buttonPin);
  if (buttonState == HIGH) {
    motorMove(input);
  }

}

void motorMove(String input) {

  //String parse begin

  Serial2.println("Data recieved");

  input.trim(); //removes spaces from string

  int lengthInput = 0;//Establish the lengthInput variable

  int lengthString = input.length();//Find the number of characters in the recieved string

  if (input.endsWith("/r")) {
    input.remove(lengthString - 2, lengthString); //Removes the carriage return (end marker) from the string, if it is present
    lengthString = input.length(); //Finds the new length of the string
  }

  if (input.endsWith("/")) {
    input.remove(lengthString - 1, lengthString);              //Removes the carriage return (end marker) from the string, if it is present
    lengthString = input.length();                             //Finds the new length of the string
  }

  if (!input.endsWith(",")) {
    if (input != "") {
      input = input + ",";                                    //Change to doesn't end with above and uncomment to add a comma
      lengthString = input.length();                            //Finds the new length of the string
    }
  }

  Serial2.print("String Processed ");
  Serial2.print(input);
  Serial2.println();

  Serial2.println("Move begin");

  //if (input != "," or "") {


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



  input = "";//Reset until the next serial data is recieved
  HasData = 1; //Indicate that there has been data recieved

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

          dxl_wb.goalPosition(j, MoveMatrix[i][j]); //Moves motor to corresponding position
          Serial2.print("Motor ID + Position"); //Print Motor ID And Pos for Debug
          Serial2.print(j);
          Serial2.print(" + ");
          Serial2.print(MoveMatrix[i][j]);
          Serial2.println();
          delay(MoveMatrix[i][0]); //This one is 0 in the matrix becase the delay is always i 0
        }
      }
    }
  }
  Serial2.println("Move end");
}
