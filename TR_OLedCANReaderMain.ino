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

#define CAN0_INT 2                              // Set INT to pin 2
MCP_CAN CAN0(10);                               // Set CS to pin 10

int counter;

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
  if(CAN0.begin(MCP_ANY, CAN_1000KBPS, MCP_8MHZ) == CAN_OK)
    display.println("MCP2515 Initialized!");
  else
    display.println("Error Initializing MCP2515...");
  display.display();
  delay(3000);
  display.clearDisplay();
  display.setCursor(0,0);
  CAN0.setMode(MCP_NORMAL);                     // Set operation mode to normal so the MCP2515 sends acks to received data.
  pinMode(CAN0_INT, INPUT);                            // Configuring pin for /INT input
  display.print("MCP2515 Library Receive Example...");
  display.display();
  delay(2000);
  display.clearDisplay();
}

void loop()
{
    //Initialize OLED Printing
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0,0);
    
    if(!digitalRead(CAN0_INT)){
      CAN0.readMsgBuf(&rxId, &len, rxBuf); // Read data: len = data length, buf = data byte(s)
      int motorID = rxId - 0x200;
      display.print("Motor #");
      display.println(motorID,HEX);
      display.println("Data:");
      /*DOES NOT WORK
      for(counter = 0; counter < 128; counter++){
        display.print(msgString[counter]);
      } HAVE TO PARSE ARRAY AND CREATE VALUES*/
    }  
      display.display();
      delay(1000);
      display.clearDisplay();
}
