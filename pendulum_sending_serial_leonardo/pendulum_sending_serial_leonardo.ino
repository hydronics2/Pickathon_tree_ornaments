// this code takes the x or y accleration (actually y and z the way I have it mounted, x is up and down)
// it takes a neutral value in setup (so pendulum should be still)
// next it subtracts from neutral the accerlation values and averages them
// it also takes a high value about every period (1.9 seconds) and reports that.
// that high value should give an idea for how windy it is.. that is; how steep the swing is.

//if the averageValue is graeter than ~2500 it sends serial to the ESP at 115200
/// data format is '[' start, COCOON#, 0-255 tap strength, ']' ending character


#include <Wire.h>
#include <SPI.h>
#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_NeoPixel.h>

#define COCCOON 1
#define PIN 6
Adafruit_NeoPixel strip = Adafruit_NeoPixel(30, PIN, NEO_GRB + NEO_KHZ800);

// I2C
Adafruit_LIS3DH lis = Adafruit_LIS3DH();


long currentTime = 0;
int lastAverageAcc = 0;

const int sizeRolling = 20;
int rollingAcc[sizeRolling];
int incrementRolling = 2;

int yNeutral = 0; 
int zNeutral = 0;   

long lastZeroHighTime = 0;

int averageHigh = 0;
int averageHighLast = 0;


void setup(void) {
  Serial.begin(115200);
  Serial1.begin(115200);
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
    long y2 = 0;
    long z2 = 0;
  for(int i = 0; i<20; i++){
    lis.read();      // get X Y and Z data at once
    int y1 = lis.y;  
    y1 = abs(y1);
    y2 = y1 + y2;
    delay(5);
    int z1 = lis.z; 
    z1 = abs(z1);
    z2 = z1 + z2;
    delay(5);
  }
  yNeutral = y2/20;
  Serial.print("yNeutral: ");
  Serial.println(yNeutral);
  Serial.print("zNeutral: ");
  zNeutral = z2/20;
  Serial.println(zNeutral); 
}


void loop() {

  currentTime = millis();
  int averageAcc = 0;

  if (Serial.available()) { //for testing... data format is '[' to start and ']' to end. sends to ESP at 115200
    byte inByte = Serial.read();
    Serial.write(inByte);
    //Serial1.write(inByte);
    Serial1.write(inByte);
  }
  

  lis.read();      // get X Y and Z data at once
  int zg = lis.z;
  zg = abs(zg);
  int yg = lis.y;
  yg = abs(yg);
  int acc = 0;
  if(yg > zg){  //this is a cheap way to see which axis is the predominent pedulum movement
    if(yg > yNeutral){
      yg = yg - yNeutral;
      acc = yg;
    }else{
      yg = yNeutral - yg;
      acc = yg;
    }
  }else{
    if(zg > zNeutral){
      zg = zg - zNeutral;
      acc = zg;
    }else{
      zg = zNeutral - zg;
      acc = yg;
    }
  }
  
  delay(2); //reading at 400hz
  rollingAcc[incrementRolling] = acc;   
  incrementRolling++;
  if(incrementRolling == sizeRolling){
    incrementRolling = 0;
    }
  findAverage();
  if(lastAverageAcc > 4000){ //TAP detected send it to ESP
    sendTapData();
  int randomColor = random(0,255);
  colorWipe(Wheel(randomColor), 0); //
  memset(rollingAcc, 0, sizeof(rollingAcc)); //clear array so it doesn't take forever to normalize
  delay(400); //
  return;
  }
  if(lastAverageAcc > averageHigh){ //keeps track of high.. shows how much the pendulum is swinging
    averageHigh = lastAverageAcc;
  }
  if(currentTime - lastZeroHighTime > 2000){ //every period 1.95 seconds at 37.5 inches, reset the average high
    lastZeroHighTime = currentTime;
    averageHighLast = averageHigh;
    averageHigh = 0;
    //Serial.println(averageHighLast);
  }
}

void findAverage(){
  long averageAcc = 0;
  for(int i=0; i<sizeRolling; i++){
    averageAcc = rollingAcc[i] + averageAcc;
  }
  lastAverageAcc = averageAcc/sizeRolling;
  Serial.println(lastAverageAcc);
}

// sends data to ESP at 115200
void sendTapData(){
  Serial1.write('['); //start value
  Serial1.write(COCCOON);
  char tapValue = map(lastAverageAcc, 2000,6000,0,255);
  Serial.print("tapValue: ");
  Serial.println(tapValue);
  Serial1.write(tapValue);
  Serial1.write(']');  //end value
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

