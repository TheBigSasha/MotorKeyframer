
String moveRead(String input) {

  for (int i = 1; i < TotalIDs; i++) {
    dxl_wb.itemWrite(i, "Torque", 0);
  }

  Serial2.println("Begin Move Read Process");

  input.trim(); //removes spaces from string

  int lengthInput;//Establish the lengthInput variable

  int lengthString = input.length();//Find the number of characters in the recieved string

  input = typeRemover(input, lengthString);

  lengthInput = input.length();//Establish the lengthInput variable

  lengthString = input.length();//Find the number of characters in the recieved string



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

  switch (recordingNow) {
    //while (recordingNow == 1) {
    case 0 : {
        int prevTime = millis();
        Serial2.println("End Rec in 15ms");
        if (millis() >= prevTime + 15 and recordingNow == 0) {
          String moveRead = moveData;
          Serial2.println(moveRead);
          return moveRead;
        }
      }
    case 1 :
      {
        startWrite:
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
        goto: startWrite
        //delayMicroseconds(1);
      }
    default:
      {
        recordingNow = 1;
        Serial2.println("Default case in recordingNow");
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
