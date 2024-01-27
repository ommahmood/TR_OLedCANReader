#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire);

int counter;

void setup() {
  Serial.begin(1000);

  Serial.println("OLED intialized");
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Address 0x3C for 128x32

  display.display();
  delay(1000);

  // Clear the buffer.
  display.clearDisplay();
  display.display();

  // text display tests
  display.setTextSize(1);
  display.setTextColor(WHITE);
}

void loop() {


  for (counter = 0 ; counter <= 25 ;  counter++) {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("test counter");
      display.print("counter: ");
      display.println(counter);
      display.display();
      delay(1000);
  }
}

