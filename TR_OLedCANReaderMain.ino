#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <mcp_can.h>

#define OLED_RESET 4 //Analog 4
#define BUTTON_ONE   5 //Digital 4
#define BUTTON_TWO   6
#define BUTTON_THREE 7
#define CAN0_INT 2 // Set INT to pin 2

MCP_CAN CAN0(10); // Set CS to pin 10
Adafruit_SSD1306 display(OLED_RESET);

//Enums for different modes
enum Modes{
  MODE_SELECTION,
  PRINT_ALL_MOTORS,
  PRINT_ONE_MOTOR
};
Modes currentMode = MODE_SELECTION;
// Variables to store Motor Data
long unsigned int rxId;
unsigned char len = 0;
unsigned char rxBuf[8];
unsigned char rxBuf1[8];
unsigned char rxBuf2[8];
unsigned char rxBuf3[8];
unsigned char rxBuf4[8];
unsigned char rxBuf5[8];
unsigned char rxBuf6[8];
unsigned char rxBuf7[8];
unsigned char rxBuf8[8];
unsigned char rxBuf9[8];
unsigned char rxBuf10[8];
unsigned char rxBuf11[8];
char msgString[128];                        // Array to store serial string
// Variables to store Motors
int selectedMotor = 0;
const int maxNumMotors = 11;
int motorIDs[maxNumMotors];
// Variables to store parsed Motor data
unsigned char angleHigh, angleLow;
unsigned char velocityHigh, velocityLow;
unsigned char torqueHigh, torqueLow;
// Variables to store complete Motor data
unsigned int angles;
signed int velocity;
signed int torque;
signed int temperature;

void setup() {
  Serial.begin(115200);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Address 0x3C for 128x32

  display.display();
  delay(1000);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);

  // Initialize MCP2515 running at 8MHz with a baudrate of 1000kb/s and the masks and filters disabled.
  if (CAN0.begin(MCP_ANY, CAN_1000KBPS, MCP_8MHZ) == CAN_OK)
    display.println("MCP2515 Initialized!");
  else
    display.println("Error Initializing MCP2515...");
  display.display();
  delay(3000);
  display.clearDisplay();
  
  display.setCursor(0, 0);
  CAN0.setMode(MCP_NORMAL);                     // Set operation mode to normal so the MCP2515 sends acks to received data.
  pinMode(CAN0_INT, INPUT);                            // Configuring pin for /INT input
  display.clearDisplay();
  
  // Initialize the motorIDs array with known motor IDs
  for(int y = 0 ; y < maxNumMotors ; y++){
    CAN0.readMsgBuf(&rxId, &len, rxBuf); // Read data: len = data length, buf = data byte(s)
    delay(500);
    CAN0.readMsgBuf(&rxId, &len, rxBuf); // Read data: len = data length, buf = data byte(s)
    motorIDs[y] = rxId-0x200;
    Serial.println(motorIDs[y], DEC);
    //rxId = 0x0;
    delay(100);
  }
  //CAN0.readMsgBuf(&rxId, &len, rxBuf); // Read data: len = data length, buf = data byte(s)
}

void loop()
{
  //Initialize OLED Printing
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  /*
  switch(currentMode){
    //*******************************************************************
    case MODE_SELECTION:
      display.println("Mode Selection");
      display.println("1. Print All Motors");
      display.println("2. Print One Motor");
      display.println("3. Back to Selection");
      while(digitalRead(BUTTON_ONE) == LOW && digitalRead(BUTTON_TWO) == LOW)
        delay(100);
      if (digitalRead(BUTTON_ONE) == HIGH) {
        currentMode = PRINT_ALL_MOTORS;
        delay(500);
      } else if (digitalRead(BUTTON_TWO) == HIGH) {
        currentMode = PRINT_ONE_MOTOR;
        delay(500);
      }
      break;
      //*******************************************************************
    case PRINT_ALL_MOTORS:
      // Display data for all motors
      for (int i = 0; i < maxNumMotors; i++) {
        selectedMotor = i;
        displayMotorData(selectedMotor);
      }

      if (digitalRead(BUTTON_THREE) == HIGH) {
        currentMode = MODE_SELECTION;
        delay(500);
      }
      break;
      //*******************************************************************
    case PRINT_ONE_MOTOR:
      displayMotorData(selectedMotor);

      if (digitalRead(BUTTON_ONE) == HIGH) {
        // Switch to the previous motor (wrap around if at the first motor)
        selectedMotor = (selectedMotor - 1 + maxNumMotors) % maxNumMotors;
        delay(500);
      } else if (digitalRead(BUTTON_TWO) == HIGH) {
        // Switch to the next motor
        selectedMotor = (selectedMotor + 1) % maxNumMotors;
        delay(500);
      } else if (digitalRead(BUTTON_THREE) == HIGH) {
        currentMode = MODE_SELECTION;
        delay(500);
      }
      break;
      //*******************************************************************
  }
  */
  CAN0.readMsgBuf(motorIDs[0], &len, rxBuf1); // Read data: len = data length, buf = data byte(s)
  CAN0.readMsgBuf(motorIDs[1], &len, rxBuf2); 
  displayMotorData(motorIDs[0]);
  displayMotorData(motorIDs[1]);
  display.display();
  delay(10);
  display.clearDisplay();
}

void displayMotorData(int motorIndex){
  //Select Motor
  //int motorID = motorIDs[motorIndex] - 0x200;
  int motorID = motorIndex;
  display.print("M#");
  display.print(motorID, DEC);
  display.print(": ");
  //display.println("Data:");
  
  //Serial Print for testing and comparing values
  sprintf(msgString, "Standard ID: 0x%.3lX       DLC: %1d  Data:", rxId, len);

  /*Serial.print(msgString);
  for(byte i = 0; i < len; i++){
      sprintf(msgString, " 0x%.2X", rxBuf[i]);
      Serial.print(msgString);
  }
  Serial.println();*/

  
  // Parse the rxBuf array for specific motor
  if(motorID == 1){
    angleHigh = rxBuf1[0];
    angleLow = rxBuf1[1];
    velocityHigh = rxBuf1[2];
    velocityLow = rxBuf1[3];
    torqueHigh = rxBuf1[4];
    torqueLow = rxBuf1[5];
    temperature = rxBuf1[6];
  
    angles = ((uint16_t)(angleHigh) << 8) | (uint16_t)(angleLow);
    velocity = ((uint16_t)(velocityHigh) << 8) | (uint16_t)(velocityLow);
    torque = ((uint16_t)(torqueHigh) << 8) | (uint16_t)(torqueLow);
  }
  else if(motorID == 2){
    angleHigh = rxBuf2[0];
    angleLow = rxBuf2[1];
    velocityHigh = rxBuf2[2];
    velocityLow = rxBuf2[3];
    torqueHigh = rxBuf2[4];
    torqueLow = rxBuf2[5];
    temperature = rxBuf2[6];
  
    angles = ((uint16_t)(angleHigh) << 8) | (uint16_t)(angleLow);
    velocity = ((uint16_t)(velocityHigh) << 8) | (uint16_t)(velocityLow);
    torque = ((uint16_t)(torqueHigh) << 8) | (uint16_t)(torqueLow);
  }
  else if(motorID == 3){
    angleHigh = rxBuf3[0];
    angleLow = rxBuf3[1];
    velocityHigh = rxBuf3[2];
    velocityLow = rxBuf3[3];
    torqueHigh = rxBuf3[4];
    torqueLow = rxBuf3[5];
    temperature = rxBuf3[6];
  
    angles = ((uint16_t)(angleHigh) << 8) | (uint16_t)(angleLow);
    velocity = ((uint16_t)(velocityHigh) << 8) | (uint16_t)(velocityLow);
    torque = ((uint16_t)(torqueHigh) << 8) | (uint16_t)(torqueLow);
  }
  else if(motorID == 4){
    angleHigh = rxBuf4[0];
    angleLow = rxBuf4[1];
    velocityHigh = rxBuf4[2];
    velocityLow = rxBuf4[3];
    torqueHigh = rxBuf4[4];
    torqueLow = rxBuf4[5];
    temperature = rxBuf4[6];
  
    angles = ((uint16_t)(angleHigh) << 8) | (uint16_t)(angleLow);
    velocity = ((uint16_t)(velocityHigh) << 8) | (uint16_t)(velocityLow);
    torque = ((uint16_t)(torqueHigh) << 8) | (uint16_t)(torqueLow);
  }
  else if(motorID == 5){
    angleHigh = rxBuf5[0];
    angleLow = rxBuf5[1];
    velocityHigh = rxBuf5[2];
    velocityLow = rxBuf5[3];
    torqueHigh = rxBuf5[4];
    torqueLow = rxBuf5[5];
    temperature = rxBuf5[6];
  
    angles = ((uint16_t)(angleHigh) << 8) | (uint16_t)(angleLow);
    velocity = ((uint16_t)(velocityHigh) << 8) | (uint16_t)(velocityLow);
    torque = ((uint16_t)(torqueHigh) << 8) | (uint16_t)(torqueLow);
  }
  else if(motorID == 6){
    angleHigh = rxBuf6[0];
    angleLow = rxBuf6[1];
    velocityHigh = rxBuf6[2];
    velocityLow = rxBuf6[3];
    torqueHigh = rxBuf6[4];
    torqueLow = rxBuf6[5];
    temperature = rxBuf6[6];
  
    angles = ((uint16_t)(angleHigh) << 8) | (uint16_t)(angleLow);
    velocity = ((uint16_t)(velocityHigh) << 8) | (uint16_t)(velocityLow);
    torque = ((uint16_t)(torqueHigh) << 8) | (uint16_t)(torqueLow);
  }
  else if(motorID == 7){
    angleHigh = rxBuf7[0];
    angleLow = rxBuf7[1];
    velocityHigh = rxBuf7[2];
    velocityLow = rxBuf7[3];
    torqueHigh = rxBuf7[4];
    torqueLow = rxBuf7[5];
    temperature = rxBuf7[6];
  
    angles = ((uint16_t)(angleHigh) << 8) | (uint16_t)(angleLow);
    velocity = ((uint16_t)(velocityHigh) << 8) | (uint16_t)(velocityLow);
    torque = ((uint16_t)(torqueHigh) << 8) | (uint16_t)(torqueLow);
  }
  else if(motorID == 8){
    angleHigh = rxBuf8[0];
    angleLow = rxBuf8[1];
    velocityHigh = rxBuf8[2];
    velocityLow = rxBuf8[3];
    torqueHigh = rxBuf8[4];
    torqueLow = rxBuf8[5];
    temperature = rxBuf8[6];
  
    angles = ((uint16_t)(angleHigh) << 8) | (uint16_t)(angleLow);
    velocity = ((uint16_t)(velocityHigh) << 8) | (uint16_t)(velocityLow);
    torque = ((uint16_t)(torqueHigh) << 8) | (uint16_t)(torqueLow);
  }
  else if(motorID == 9){
    angleHigh = rxBuf9[0];
    angleLow = rxBuf9[1];
    velocityHigh = rxBuf9[2];
    velocityLow = rxBuf9[3];
    torqueHigh = rxBuf9[4];
    torqueLow = rxBuf9[5];
    temperature = rxBuf9[6];
  
    angles = ((uint16_t)(angleHigh) << 8) | (uint16_t)(angleLow);
    velocity = ((uint16_t)(velocityHigh) << 8) | (uint16_t)(velocityLow);
    torque = ((uint16_t)(torqueHigh) << 8) | (uint16_t)(torqueLow);
  }
  else if(motorID == 10){
    angleHigh = rxBuf10[0];
    angleLow = rxBuf10[1];
    velocityHigh = rxBuf10[2];
    velocityLow = rxBuf10[3];
    torqueHigh = rxBuf10[4];
    torqueLow = rxBuf10[5];
    temperature = rxBuf10[6];
  
    angles = ((uint16_t)(angleHigh) << 8) | (uint16_t)(angleLow);
    velocity = ((uint16_t)(velocityHigh) << 8) | (uint16_t)(velocityLow);
    torque = ((uint16_t)(torqueHigh) << 8) | (uint16_t)(torqueLow);
  }
  else if(motorID == 11){
    angleHigh = rxBuf11[0];
    angleLow = rxBuf11[1];
    velocityHigh = rxBuf11[2];
    velocityLow = rxBuf11[3];
    torqueHigh = rxBuf11[4];
    torqueLow = rxBuf11[5];
    temperature = rxBuf11[6];
  
    angles = ((uint16_t)(angleHigh) << 8) | (uint16_t)(angleLow);
    velocity = ((uint16_t)(velocityHigh) << 8) | (uint16_t)(velocityLow);
    torque = ((uint16_t)(torqueHigh) << 8) | (uint16_t)(torqueLow);
  }
  display.print("A: ");
  display.print(angles, DEC);

  display.print(", V: ");
  display.println(velocity, DEC);

  display.print("Tor: ");
  display.print(torque, DEC);

  display.print(", Temp: ");
  display.println(temperature, DEC);
  //display.println("****************");
}
