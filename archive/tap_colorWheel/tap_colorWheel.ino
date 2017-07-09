// Basic demo for tap/doubletap readings from Adafruit LIS3DH

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>

Adafruit_LIS3DH lis = Adafruit_LIS3DH();

#include <Adafruit_NeoPixel.h>

#define PIN 6
Adafruit_NeoPixel strip = Adafruit_NeoPixel(25, PIN, NEO_GRB + NEO_KHZ800);


// Adjust this number for the sensitivity of the 'click' force
// this strongly depend on the range! for 16G, try 5-10
// for 8G, try 10-20. for 4G try 20-40. for 2G try 40-80
#define CLICKTHRESHHOLD 20

void setup(void) {

  Serial.begin(9600);
    Serial.println("Adafruit LIS3DH Tap Test!");
  
  if (! lis.begin(0x18)) {   // change this to 0x19 for alternative i2c address
    Serial.println("Couldnt start");
    while (1);
  }
  Serial.println("LIS3DH found!");
  
  lis.setRange(LIS3DH_RANGE_2_G);   // 2, 4, 8 or 16 G!
  
  Serial.print("Range = "); Serial.print(2 << lis.getRange());  
  Serial.println("G");

  // 0 = turn off click detection & interrupt
  // 1 = single click only interrupt output
  // 2 = double click only interrupt output, detect single click
  // Adjust threshhold, higher numbers are less sensitive
  lis.setClick(2, CLICKTHRESHHOLD);
  delay(100);

    strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  
}


void loop() {
  uint8_t click = lis.getClick();
  if (click == 0) return;
  if (! (click & 0x30)) return;
  Serial.print("Click detected (0x"); Serial.print(click, HEX); Serial.print("): ");
  if (click & 0x10) Serial.print(" single click");
  if (click & 0x20) Serial.print(" double click");
  Serial.println();
  
  int randomColor = random(0,255);
  colorWipe(Wheel(randomColor), 0); // Red
  delay(100);
  return;
}


// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

