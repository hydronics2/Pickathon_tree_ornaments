// Basic demo for accelerometer readings from Adafruit LIS3DH

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_NeoPixel.h>


#define PIN 6
Adafruit_NeoPixel strip = Adafruit_NeoPixel(5, PIN, NEO_GRB + NEO_KHZ800);

// I2C
Adafruit_LIS3DH lis = Adafruit_LIS3DH();


float xg, yg, zg;
long currentTime = 0;
int lastAverageAcc = 0;
int intensity = 60;

const int sizeRolling = 20;
const int sizeRollingLarge = 80;
int rollingAcc[sizeRolling];
int rollingLarge[sizeRollingLarge];
int incrementRolling = 2;
int incrementRollingLarge = 0;

int lastAverageLarge = 0;

int xLastAverageAcc = 0;

int sumIncrement = 0;
int goingUpFlag = 0;
long lastTimeLow = 0;
long lastTimeHigh = 0;

float xNeutral = 0;    

long lastIncrementLarge = 0;
long lastUpdateTime = 0;


void setup(void) {
  Serial.begin(115200);
  Serial.println("LIS3DH test!");
  if (! lis.begin(0x18)) {   // change this to 0x19 for alternative i2c address
    Serial.println("Couldnt start");
    while (1);
  }
  Serial.println("LIS3DH found!");
  
  lis.setRange(LIS3DH_RANGE_2_G);   // 2, 4, 8 or 16 G!

  //1, 10, 25, 50, 100, 200, 400_HZ, LOWPOWER_1K6HZ, _5KHZ
  lis.setDataRate(LIS3DH_DATARATE_400_HZ);
  
  Serial.print("Range = "); Serial.print(2 << lis.getRange());  
  Serial.println("G");

  Serial.print("Data rate = "); Serial.print(lis.getDataRate());  
  Serial.println("Hz");

  strip.begin();
  strip.show(); // Initialize all pixels to 'off'



  //capture the near neutral position of the 200G accerometer..at 12bit or 50% of 0-4095=2048
    long x2 = 0;
    int y2 = 0;
    int z2 = 0;
  for(int i = 0; i<20; i++){

    lis.read();      // get X Y and Z data at once
    // Then print out the raw data
    //Serial.print("X:  "); Serial.print(lis.x); 
    //Serial.print("  \tY:  "); Serial.print(lis.y); 
    //Serial.print("  \tZ:  "); Serial.print(lis.z); 
  
    int x1 = lis.y;  //read and throw away
    //delay(5);
    //x1 = analogRead(xAxis);
    x2 = x1 + x2;
    delay(5);
    //int y1 = analogRead(yAxis);
    //y2 = y1 + y2;
    //delay(5);
    //int z1 = analogRead(zAxis);
    //z2 = z1 + z2;
    //delay(5);
  }
  xNeutral = x2/20;
  Serial.println(xNeutral);
  //yNeutral = (int)y2/20;
  //Serial.println(yNeutral);
  //zNeutral = (int)z2/20;
  //Serial.println(zNeutral); 






  
}


void loop() {

  currentTime = millis();
  int averageAcc = 0;

  lis.read();      // get X Y and Z data at once
  // Then print out the raw data
  //Serial.print("X:  "); 
  //Serial.println(lis.x); 
  xg = lis.y;
  
  /* Display the results (acceleration is measured in m/s^2) */
  //xg = event.acceleration.x;
  //yg = event.acceleration.y;
  //zg = event.acceleration.z;
  //int totAcc = 100* sqrt(yg*yg + zg*zg); //9.3 instead of 9.81 based on acc values
  //int totAcc = 10* sqrt(xg*xg); //
  if(xg > xNeutral){
    xg = xg - xNeutral;
  }else{
    xg = xNeutral - xg;
  }
    

  totAcc = xg;

  delay(2);
  rollingAcc[incrementRolling] = totAcc;      //keeps track of thrown or dropped
  incrementRolling++;
  if(incrementRolling == sizeRolling){
    incrementRolling = 0;
    }
  findAverage();
  
  if(currentTime - lastIncrementLarge > 50){
    lastIncrementLarge = currentTime;
    rollingLarge[incrementRollingLarge] = totAcc;      //keeps track of thrown or dropped
    incrementRollingLarge++;
    if(incrementRollingLarge == sizeRollingLarge){
      incrementRollingLarge = 0;
      findAverageLarge();
      }
  }

}

void findAverageLarge(){
  int high = 0;
  long averageAcc = 0;
  for(int i=0; i<sizeRollingLarge; i++){
    averageAcc = rollingLarge[i] + averageAcc;
  }
  lastAverageLarge = averageAcc/sizeRollingLarge;
}
  
void findAverage(){
  long averageAcc = 0;
  for(int i=0; i<sizeRolling; i++){
    averageAcc = rollingAcc[i] + averageAcc;
  }
  lastAverageAcc = averageAcc/sizeRolling;
  Serial.println(lastAverageAcc);
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

