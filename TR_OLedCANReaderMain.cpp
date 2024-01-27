#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire);

int counter;

void setup() {
  Serial.begin(1000);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Address 0x3C for 128x32

  display.display();
  delay(1000);

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);

  attachInterrupt(0, MCP2525_ISR, FAILING);
  void MCP2515_ISR() {
  Flag_Recv = 1;
  }
  

  
}

void loop()
{
    if(Flag_Recv)                   // check if data was recieved
    {
      Flag_Recv = 0;                // clear flag
      CAN.readMsgBuf(&len, buf);    // read data,  len: data length, buf: data buf
      canId = CAN.getCanId();
      //Print data to the serial console 
      //and the LCD display
      Serial.println("CAN_BUS GET DATA!");
      Serial.print("CAN ID: ");
      Serial.println(canId);
      lcd.setCursor(0, 0);
      lcd.print("CAN ID: ");
      lcd.print(canId);
      lcd.setCursor(0, 2);
      Serial.print("data len = ");Serial.println(len);
      //This loops through each byte of data and prints it
      for(int i = 0; i<len; i++)    // print the data
      {
        Serial.print(buf[i]);Serial.print("\t");
        lcd.print(buf[i]);
        lcd.print(" ");
      }
      Serial.println();
      delay(50);
    }
}
