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

#define COCCOON 9
#define PIN 6
Adafruit_NeoPixel strip = Adafruit_NeoPixel(15, PIN, NEO_GRB + NEO_KHZ800);

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

int serialChar = 0;
int serialInput[26];
boolean stringComplete = false;
long lastTimeSent = 0;



void setup(void) {
  Serial.begin(115200);
  Serial1.begin(115200);
//  Serial.println("LIS3DH test!");
//  if (! lis.begin(0x18)) {   // change this to 0x19 for alternative i2c address
//    Serial.println("Couldnt start");
//    while (1);
//  }
//  Serial.println("LIS3DH found!");
//  
//  lis.setRange(LIS3DH_RANGE_2_G);   // 2, 4, 8 or 16 G!
//
//  //1, 10, 25, 50, 100, 200, 400_HZ, LOWPOWER_1K6HZ, _5KHZ
//  lis.setDataRate(LIS3DH_DATARATE_400_HZ);
//  
//  Serial.print("Range = "); Serial.print(2 << lis.getRange());  
//  Serial.println("G");
//
//  Serial.print("Data rate = "); Serial.print(lis.getDataRate());  
//  Serial.println("Hz");
//
//  strip.begin();
//  strip.show(); // Initialize all pixels to 'off'
//
//  //capture the near neutral position of the 200G accerometer..at 12bit or 50% of 0-4095=2048
//    long y2 = 0;
//    long z2 = 0;
//  for(int i = 0; i<20; i++){
//    lis.read();      // get X Y and Z data at once
//    int y1 = lis.y;  
//    y1 = abs(y1);
//    y2 = y1 + y2;
//    delay(5);
//    int z1 = lis.z; 
//    z1 = abs(z1);
//    z2 = z1 + z2;
//    delay(5);
//  }
//  yNeutral = y2/20;
//  Serial.print("yNeutral: ");
//  Serial.println(yNeutral);
//  Serial.print("zNeutral: ");
//  zNeutral = z2/20;
//  Serial.println(zNeutral); 
}


void loop() {

  
//  lastAverageAcc = 3000;
//  
//  if(currentTime - lastTimeSent > 5000){
//    sendTapData();
//    lastTimeSent = currentTime;
//  }
  
  
  serialEvent();  //look for Serial data coming in...

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
// sending Coccoon#, Red,Green,Blue, Intensity
void sendTapData(){
  Serial1.write('['); //start value
  Serial.println('['); //start value
  
  Serial1.write(COCCOON);
  Serial.println(COCCOON);
  
  byte randomNumber = (random(0,255));
  Serial1.write(randomNumber); //red
  Serial.println(randomNumber); //red
  randomNumber = (random(0,255));
  Serial1.write(randomNumber); //green
  Serial.println(randomNumber); //green
  randomNumber = (random(0,255));
  Serial1.write(randomNumber); //blue  
  Serial.println(randomNumber); //blue
  
  char tapValue = map(lastAverageAcc, 2000,6000,0,255); 
  Serial1.write(tapValue); //tap value
  Serial.println(tapValue); //tap value
  
  Serial1.write(']');  //end value
  Serial.println(']');  //end value
  Serial.println("");
}

//--------------------------------------------------------- SERIAL EVENT - UDP Broadcast from Server
//Example Data: 91,123,228,43,13,25,24,22,21,19,18,16,15,14,12,11,9,8,7,5,4,2,1,0,1,
// 91 is indicates the beginning of the data
//Next Three Bytes are RGB - 123,228,43
//Next Byte is Intensity - 13 (0-254)
//Next 20 bytes are Distance from Pendulum that was tapped, 
//where the 1st byte is the distance between pendulum 1 and the pendulum that was tapped 
//... in this case the tapped pendulum is #19 and it is a distance of 25 from the 1st pendulum
//Distance is measured in 10ths of feet so, 25 equates to 2.5 feet.


void serialEvent() {
  while (Serial1.available()) {
    byte inChar = (byte)Serial1.read();
    //Serial.println(inChar);
    if(inChar == '['){
      Serial.println("data from Server");
      serialChar = 0;
    }else if(serialChar < 25){
      Serial.println(inChar);
      serialInput[serialChar] = inChar;
      serialChar++;
    }
  }
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

