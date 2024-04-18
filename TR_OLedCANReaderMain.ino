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
int numMotors = 0; // Actual number of motors connected
unsigned long lastDebounceTime = 0; // Last time buttons' states were checked

long unsigned int rxId;
unsigned char len = 0;
unsigned char rxBuf[8];
char msgString[128];                        // Array to store serial string
// Struct to store parsed data
unsigned char angleHigh, angleLow;
unsigned char velocityHigh, velocityLow;
unsigned char torqueHigh, torqueLow;

// Variables to store complete data
struct MotorData {
  unsigned long id;
  unsigned int angles;
  signed int velocity;
  signed int torque;
  signed int temperature;
};
// Function Declarations

const int MAX_MOTORS = 11; // The maximum number of motors

MotorData motors[MAX_MOTORS]; // Array to hold data for each motor
int motorIDs[MAX_MOTORS]; // Array to hold the unique IDs for each motor

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
  initializeMotorData(); // Initialize the motor data structures
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
            currentPage = 0; // Start from the first motor
            lastDebounceTime = currentMillis; // Update the last debounce time
          }
          displayModeSelection();
          break;
        case PRINT_ALL_MOTORS:
          if (digitalRead(BUTTON_ONE) == HIGH) {
            navigatePages(-1); // Navigate backward
            Serial.print("Page is now: ");
            Serial.println(currentPage);
            lastDebounceTime = currentMillis; // Update the last debounce time
          } else if (digitalRead(BUTTON_TWO) == HIGH) {
            navigatePages(1); // Navigate forward
            Serial.print("Page is now: ");
            Serial.println(currentPage);
            lastDebounceTime = currentMillis; // Update the last debounce time
          }
          displayAllMotors();
          break;
        case PRINT_ONE_MOTOR:
          if (digitalRead(BUTTON_ONE) == HIGH) {
            navigatePages(-1); // Navigate backward
            Serial.print("Page is now: ");
            Serial.println(currentPage);
            lastDebounceTime = currentMillis; // Update the last debounce time
          } else if (digitalRead(BUTTON_TWO) == HIGH) {
            navigatePages(1); // Navigate forward
            Serial.print("Page is now: ");
            Serial.println(currentPage);
            lastDebounceTime = currentMillis; // Update the last debounce time
          }
          displayOneMotor();
          break;
      }
    }
  }
}


void displayMotorDataAll(int motorIndex){
  // Directly display the motor ID, with any necessary adjustments for display purposes.
  // Subtract 0x200 from id for display.
  unsigned long displayID1 = motors[motorIndex].id - 0x200;
  unsigned long displayID2 = motors[motorIndex+1].id - 0x200;
  
  Serial.print("MOTOR IDs DISPLAYED ARE: ");
  Serial.println(displayID1);
  Serial.println(displayID2);
  display.clearDisplay();
  display.setCursor(0, 0);

  Serial.println();
    //Print first motor
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("M");
    display.print(displayID1); 
    display.print("|A:");
    display.print(motors[motorIndex].angles, DEC);
    display.print(", V:");
    display.println(motors[motorIndex].velocity, DEC);
    display.print("Tor: ");
    display.print(motors[motorIndex].torque, DEC);
    display.print(", Temp: ");
    display.println(motors[motorIndex].temperature, DEC);
    
    //Print second motor
    display.print("M");
    display.print(displayID2); 
    display.print("|A:");
    display.print(motors[motorIndex+1].angles, DEC);
    display.print(", V:");
    display.println(motors[motorIndex+1].velocity, DEC);
    display.print("Tor: ");
    display.print(motors[motorIndex+1].torque, DEC);
    display.print(", Temp: ");
    display.println(motors[motorIndex+1].temperature, DEC);
    display.display();
}

void displayMotorDataOne(int motorIndex){
  if (motorIndex < 0 || motorIndex >= MAX_MOTORS) {
    Serial.println("Invalid motor index.");
     Serial.println(motorIndex);
    return; // Ensure the motorIndex is within a valid range.
  }

  // Check if the motor slot is uninitialized using (unsigned long)-1 as the sentinel value.
  if (motors[motorIndex].id == (unsigned long)-1) {
    Serial.println("Motor slot is uninitialized.");
    return; // Skips displaying data for uninitialized slots.
  }

  // Directly display the motor ID, with any necessary adjustments for display purposes.
  // Subtract 0x200 from id for display.
  unsigned long displayID = motors[motorIndex].id - 0x200;

  display.clearDisplay();
  display.setCursor(0, 0);

  // Display the motor data, starting with the adjusted ID for readability.
  display.print("Motor#");
  display.println(displayID); 
  display.print("A: ");
  display.print(motors[motorIndex].angles, DEC); // Display angle
  display.print(", V: ");
  display.println(motors[motorIndex].velocity, DEC); // Display velocity
  display.print("Tor: ");
  display.print(motors[motorIndex].torque, DEC); // Display torque
  display.print(", Temp: ");
  display.println(motors[motorIndex].temperature, DEC); // Display temperature

  display.display(); // Refresh the OLED display with the new data
}


void navigatePages(int page) {
  if (currentMode == PRINT_ALL_MOTORS) {
    // Logic for navigating pages in PRINT_ALL_MOTORS mode
    int totalPages = (numMotors + 1) / 2; // Assuming 2 motors per page
    currentPage += page;
    if (currentPage < 1) 
      currentPage = totalPages;
    else if (currentPage > totalPages) 
      currentPage = 1;
  } else if (currentMode == PRINT_ONE_MOTOR) {
      // Logic for navigating motors in PRINT_ONE_MOTOR mode
      int totalPages = numMotors + 1;
      currentPage += page;
      if (currentPage < 1) 
        currentPage = totalPages;
      else if (currentPage > totalPages) 
        currentPage = 1;
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
  int motorIndex = currentPage - 1; // Calculate start index based on currentPage
  display.clearDisplay();
  display.setCursor(0, 0);
  if(numMotors > 1){
    displayMotorDataAll(motorIndex);
  }
  else
    displayMotorDataAll(motorIndex);
  display.display(); // Refresh the display to show the new page
}

void displayOneMotor() {
  display.clearDisplay();
  display.setCursor(0, 0);
  motorIndex = currentPage; // Calculate motor index based on currentPage
  if (motorIndex <= numMotors) { // Check to avoid accessing beyond the array
    displayMotorDataOne(motorIndex);
    display.display(); // Refresh the display to show the selected motor's data
  }
}

void readCANMessage() {
  if (CAN0.checkReceive() == CAN_MSGAVAIL) {
    CAN0.readMsgBuf(&rxId, &len, rxBuf);
    parseMotorData(rxBuf, rxId);
    addUniqueMotorID(rxId);
  }
}


bool addUniqueMotorID(unsigned long id) {
  // Skip adding if the ID is 0x200
  if (id == 0x200 || id > 0x215) 
    return false;

  for (int i = 0; i < MAX_MOTORS; i++) {
    if (motors[i].id == id) {
      return false; // ID already present, no need to add
    }
    if (motors[i].id == (unsigned long)-1) { // Check for an empty slot
      motors[i].id = id;
      return true; // New ID added
    }
  }
  return false; // No space left to add a new ID
}

void initializeMotorData() {
  for (int i = 0; i < MAX_MOTORS; i++) {
    motors[i].id = (unsigned long)-1; 
    motors[i].angles = 0;
    motors[i].velocity = 0;
    motors[i].torque = 0;
    motors[i].temperature = 0;
  }
}


void parseMotorData(unsigned char* info, unsigned long motorID) {
  // Find the motor index based on motorID
  int index = -1;
  for (int i = 0; i < MAX_MOTORS; i++) {
    if (motors[i].id == motorID) {
      index = i;
      break;
    }
  }
  if (index != -1) {
    // Debug: Print the motorID and index being updated
    Serial.print("Updating motor at index ");
    Serial.print(index);
    Serial.print(" with ID ");
    Serial.println(motorID, HEX); // Assuming motorID is in HEX format

    // Parse and update motor data
    motors[index].angles = ((uint16_t)(info[0]) << 8) | (uint16_t)(info[1]);
    motors[index].velocity = ((uint16_t)(info[2]) << 8) | (uint16_t)(info[3]);
    motors[index].torque = ((uint16_t)(info[4]) << 8) | (uint16_t)(info[5]);
    motors[index].temperature = info[6];

    // Debug: Print parsed values
    Serial.print("Angles: ");
    Serial.print(motors[index].angles);
    Serial.print(", Velocity: ");
    Serial.print(motors[index].velocity);
    Serial.print(", Torque: ");
    Serial.print(motors[index].torque);
    Serial.print(", Temperature: ");
    Serial.println(motors[index].temperature);
  } else {
    Serial.println("Motor ID not found or array full."); // In case index is not found
  }
}
