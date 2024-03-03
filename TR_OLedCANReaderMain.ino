#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <mcp_can.h>

#define OLED_RESET 4 //Analog 4
#define BUTTON_ONE   4 //Digital 4
#define BUTTON_TWO   5
#define BUTTON_THREE 6
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
  motorIDs[0]  = 0x201;
  motorIDs[1]  = 0x202;
  motorIDs[2]  = 0x203;
  motorIDs[3]  = 0x204;
  motorIDs[4]  = 0x205;
  motorIDs[5]  = 0x206;
  motorIDs[6]  = 0x207;
  motorIDs[7]  = 0x208;
  motorIDs[8]  = 0x209;
  motorIDs[9]  = 0x20A;
  motorIDs[10] = 0x20B;
}

void loop()
{
  //Initialize OLED Printing
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  
  switch(currentMode){
    //*******************************************************************
    case MODE_SELECTION:
      display.println("Mode Selection");
      display.println("1. Print All Motors");
      display.println("2. Print One Motor");
      display.println("3. Back to Selection");

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
  break;
  }
  
  display.display();
  delay(10);
  display.clearDisplay();
}

void displayMotorData(int motorIndex){
  //Select Motor
  int motorID = motorIDs[motorIndex] - 0x200;
  
  display.print("Motor #");
  display.println(motorID, DEC);
  display.println("Data:");
  
  //Serial Print for testing and comparing values
  sprintf(msgString, "Standard ID: 0x%.3lX       DLC: %1d  Data:", rxId, len);

  Serial.print(msgString);
  for(byte i = 0; i < len; i++){
      sprintf(msgString, " 0x%.2X", rxBuf[i]);
      Serial.print(msgString);
  }
  Serial.println();

  
  // Parse the rxBuf array
  angleHigh = rxBuf[0];
  angleLow = rxBuf[1];
  velocityHigh = rxBuf[2];
  velocityLow = rxBuf[3];
  torqueHigh = rxBuf[4];
  torqueLow = rxBuf[5];
  temperature = rxBuf[6];

  angles = ((uint16_t)(angleHigh) << 8) | (uint16_t)(angleLow);
  velocity = ((uint16_t)(velocityHigh) << 8) | (uint16_t)(velocityLow);
  torque = ((uint16_t)(torqueHigh) << 8) | (uint16_t)(torqueLow);
 
  display.print("A: ");
  display.print(angles, DEC);

  display.print(", V: ");
  display.println(velocity, DEC);

  display.print("Tor: ");
  display.print(torque, DEC);

  display.print(", Temp: ");
  display.println(temperature, DEC);
}
