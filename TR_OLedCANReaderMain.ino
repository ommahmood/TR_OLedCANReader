#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <mcp_can.h>

#define OLED_RESET 4
#define BUTTON_ONE 5
#define BUTTON_TWO 6
#define BUTTON_THREE 7
#define CAN0_INT 2
#define DEBOUNCE_DELAY 100 // Button debounce delay in milliseconds

MCP_CAN CAN0(10); // Set CS to pin 10 for MCP2515
Adafruit_SSD1306 display(OLED_RESET);

enum Modes { MODE_SELECTION, PRINT_ALL_MOTORS, PRINT_ONE_MOTOR };
Modes currentMode = MODE_SELECTION;
int motorIndex;

int currentPage = 1;
int motorIDs[11] = {0};
int numMotors = 0; // Actual number of motors connected
unsigned long lastDebounceTime = 0; // Last time buttons' states were checked

long unsigned int rxId;
unsigned char len = 0;
unsigned char rxBuf[8];
char msgString[128];                        // Array to store serial string
// Variables to store parsed data
unsigned char angleHigh, angleLow;
unsigned char velocityHigh, velocityLow;
unsigned char torqueHigh, torqueLow;
// Variables to store complete data
unsigned int angles;
signed int velocity;
signed int torque;
signed int temperature;

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_ONE, INPUT);
  pinMode(BUTTON_TWO, INPUT);
  pinMode(BUTTON_THREE, INPUT);
  
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.display();
  delay(2000); // Pause for 2 seconds
  display.clearDisplay();
  
  // Initialize MCP2515
  if (CAN0.begin(MCP_ANY, CAN_1000KBPS, MCP_8MHZ) == CAN_OK) Serial.println(F("MCP2515 Initialized Successfully!"));
  else {
    Serial.println("Error Initializing MCP2515...");
    while (1);
  }
  CAN0.setMode(MCP_NORMAL); // Set operation mode to normal so the MCP2515 sends acks to received data.
  
  display.setTextSize(1);
  display.setTextColor(WHITE);
  CAN0.setMode(MCP_NORMAL);                     // Set operation mode to normal so the MCP2515 sends acks to received data.
  pinMode(CAN0_INT, INPUT);                            // Configuring pin for /INT input
  display.clearDisplay();
}

void loop() {
  readCANMessage(); // Continuously read CAN messages to update motor IDs
  unsigned long currentMillis = millis();
  
  if (currentMillis - lastDebounceTime > DEBOUNCE_DELAY) {
    if (digitalRead(BUTTON_THREE) == HIGH) {
      // Use Button Three to return to mode selection from any mode
      if (currentMode != MODE_SELECTION) {
        currentMode = MODE_SELECTION; // Return to mode selection
        lastDebounceTime = currentMillis; // Update the last debounce time
      }
    } else {
      switch (currentMode) {
        case MODE_SELECTION:
          if (digitalRead(BUTTON_ONE) == HIGH) {
            currentMode = PRINT_ALL_MOTORS; // Enter PRINT_ALL_MOTORS mode
            currentPage = 1; // Start from the first page
            lastDebounceTime = currentMillis; // Update the last debounce time
          } else if (digitalRead(BUTTON_TWO) == HIGH) {
            currentMode = PRINT_ONE_MOTOR; // Enter PRINT_ONE_MOTOR mode
            currentPage = 1; // Start from the first motor
            lastDebounceTime = currentMillis; // Update the last debounce time
          }
          displayModeSelection();
          break;
        case PRINT_ALL_MOTORS:
          if (digitalRead(BUTTON_ONE) == HIGH) {
            navigatePages(-1); // Navigate backward
            lastDebounceTime = currentMillis; // Update the last debounce time
          } else if (digitalRead(BUTTON_TWO) == HIGH) {
            navigatePages(1); // Navigate forward
            lastDebounceTime = currentMillis; // Update the last debounce time
          }
          displayAllMotors();
          break;
        case PRINT_ONE_MOTOR:
          if (digitalRead(BUTTON_ONE) == HIGH) {
            navigatePages(-1); // Navigate backward
            lastDebounceTime = currentMillis; // Update the last debounce time
          } else if (digitalRead(BUTTON_TWO) == HIGH) {
            navigatePages(1); // Navigate forward
            lastDebounceTime = currentMillis; // Update the last debounce time
          }
          displayOneMotor();
          break;
      }
    }
  }
}


void displayMotorDataAll(int motorIndex){
  //Select Motor
  CAN0.readMsgBuf(motorIDs[motorIndex], &len, rxBuf);
  int motorID = motorIDs[motorIndex] - 0x200;
  
  display.print("M");
  display.print(motorID, DEC);
  display.print("|");
  
  //Serial Print for testing and comparing values
  sprintf(msgString, "Standard ID: 0x%.3lX       DLC: %1d  Data:", motorIDs[motorIndex], len);
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

void displayMotorDataOne(int motorIndex){
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

void navigatePages(int direction) {
  if (currentMode == PRINT_ALL_MOTORS) {
    // Logic for navigating pages in PRINT_ALL_MOTORS mode
    int totalPages = (numMotors + 1) / 2; // Assuming 2 motors per page
    currentPage += direction;
    if (currentPage < 1) currentPage = totalPages;
    else if (currentPage > totalPages) currentPage = 1;
  } else if (currentMode == PRINT_ONE_MOTOR) {
    // Logic for navigating motors in PRINT_ONE_MOTOR mode
    currentPage += direction;
    if (currentPage < 1) currentPage = numMotors;
    else if (currentPage > numMotors) currentPage = 1;
  }
}

void displayModeSelection() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(F("Select Mode:"));
  display.println(F("1. All Motors"));
  display.println(F("2. One Motor"));
  display.display(); // Ensure to refresh the display after updates
}

void displayAllMotors() {
  // Assuming 2 motors per page for simplicity
  int startIndex = (currentPage - 1) * 2; // Calculate start index based on currentPage
  display.clearDisplay();
  display.setCursor(0, 0);
  for (int i = startIndex; i < startIndex + 2 && i < numMotors; i++) {
    displayMotorDataAll(i);
  }
  display.display(); // Refresh the display to show the new page
}

void displayOneMotor() {
  motorIndex = currentPage - 1; // Calculate motor index based on currentPage
  if (motorIndex < numMotors) { // Check to avoid accessing beyond the array
    display.clearDisplay();
    display.setCursor(0, 0);
    displayMotorDataOne(motorIndex);
    display.display(); // Refresh the display to show the selected motor's data
  }
}

void readCANMessage() {
  long unsigned int rxId;
  unsigned char len;
  unsigned char rxBuf[8];
  
  if (CAN0.checkReceive() == CAN_MSGAVAIL) {
    CAN0.readMsgBuf(&rxId, &len, rxBuf);
    addUniqueMotorID(rxId);
  }
}

bool addUniqueMotorID(long unsigned int id) {
  // Simplified logic: directly use the CAN ID as motor ID
  int motorID = id;
  
  for (int i = 0; i < numMotors; i++) {
    if (motorIDs[i] == motorID) return false; // Already in array
  }
  if (numMotors < 11) {
    motorIDs[numMotors++] = motorID; // Add new ID and increment count
    return true;
  }
  return false; // Array full, no new ID added
}
