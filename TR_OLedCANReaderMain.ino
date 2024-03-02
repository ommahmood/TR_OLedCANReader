#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <mcp_can.h>

#define OLED_RESET 4

Adafruit_SSD1306 display(OLED_RESET);
//Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire);

long unsigned int rxId;
unsigned char len = 0;
unsigned char rxBuf[8];
char msgString[128];                        // Array to store serial string

// Variables to store parsed data
unsigned char angleHigh, angleLow;
unsigned char velocityHigh, velocityLow;
unsigned char torqueHigh, torqueLow;
//unsigned char temperatureHigh, temperatureLow;
// Variables to store complete data
unsigned int angles;
signed int velocity;
signed int torque;
signed int temperature;
#define CAN0_INT 2                              // Set INT to pin 2
MCP_CAN CAN0(10);                               // Set CS to pin 10

int counter;

//Enums for different modes
enum Modes{
  MODE_SELECTION,
  PRINT_ALL_MOTORS,
  PRINT_ONE_MOTOR
};

Modes currentMode = MODE_SELECTION;
int selectedMotor = 0;

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
}

void loop()
{
  //Initialize OLED Printing
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  if (!digitalRead(CAN0_INT)) {
  CAN0.readMsgBuf(&rxId, &len, rxBuf); // Read data: len = data length, buf = data byte(s)
  int motorID = rxId - 0x200;
  displayMotorData(motorID);
  display.display();
  delay(10);
  display.clearDisplay();
  }
}

void displayMotorData(int motorID){
    display.print("Motor #");
    display.println(motorID, DEC);
    display.println("Data:");
    
    /*for(counter = 0; counter < 128; counter++){
    display.print(rxBuf[counter], HEX);
    display.print(", ");
    } */
    //Serial Print for testing and comparing values
    sprintf(msgString, "Standard ID: 0x%.3lX       DLC: %1d  Data:", rxId, len);
  
    Serial.print(msgString);
    for(byte i = 0; i<len; i++){
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
    //temperature = ((uint16_t)(temperatureHigh) << 8) | (uint16_t)(temperatureLow);
    display.print("A: ");
    display.print(angles, DEC);

    display.print(", V: ");
    display.println(velocity, DEC);

    display.print("Tor: ");
    display.print(torque, DEC);

    display.print(", Temp: ");
    display.println(temperature, DEC);
  }
