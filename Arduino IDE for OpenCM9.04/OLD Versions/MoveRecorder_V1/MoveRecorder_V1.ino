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
DynamixelWorkbench dxl_wb;
String input;
bool hasData;
void setup() { // SAME AS Serial-To-Array-To-Move V10
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
    //============================= Debugging Readout ================================
    Serial2.print("Motor ");
    Serial2.print(i);
    Serial2.print(" initialized");
    Serial2.println();
  }
}












//THE CODE BELOW IS NEW RELATIVE TO Serial-To-Array-To-Move v10



int recordingButton;
#define recordButton 21


void loop() {
  String nuveau = Serial2.readStringUntil('/r');
  if (nuveau != input and nuveau != "" and nuveau != "^M" and hasData == 0) {
    if (nuveau.startsWith("<readMove>")) {
      int sampleFPS = 24;
      int sampleFrames = 240;
      input = moveRead(sampleFPS, sampleFrames);
      hasData = 1;
      Serial2.print("String recieved, press button to move");
      Serial2.print(input);
      Serial2.println();
      Serial2.flush();
      nuveau = "";
    }
  }

}

String moveRead(int sampleFPS, int sampleFrames) {

  int sampleMSPF = 1000 / sampleFPS;
  int sampleTimeMs = sampleFrames * sampleMSPF;
  String moveData;
  int startTime = millis();
  int previousTime = startTime;
  bool recordingNow = 1;
  while (recordingNow == 1) {
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
    }
    //delayMicroseconds(1);
  }
  if (recordingNow == 0) {
    int prevTime = millis();
    Serial2.println(moveData);
    if (millis() >= prevTime + 15) {
      String moveRead = moveData;
      return moveRead;
    }
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
