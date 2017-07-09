// Basic demo for accelerometer readings from Adafruit LIS3DH

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_NeoPixel.h>


#define PIN 6
Adafruit_NeoPixel strip = Adafruit_NeoPixel(10, PIN, NEO_GRB + NEO_KHZ800);

// I2C
Adafruit_LIS3DH lis = Adafruit_LIS3DH();


double xg, yg, zg;
long currentTime = 0;
int lastAverageAcc = 0;
int intensity = 60;

const int sizeRolling = 30;
int rollingAcc[sizeRolling];
int incrementRolling = 2;

int xLastAverageAcc = 0;

int sumIncrement = 0;
int goingUpFlag = 0;
long lastTimeLow = 0;
long lastTimeHigh = 0;
    


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
  lis.setDataRate(LIS3DH_DATARATE_200_HZ);
  
  Serial.print("Range = "); Serial.print(2 << lis.getRange());  
  Serial.println("G");

  Serial.print("Data rate = "); Serial.print(lis.getDataRate());  
  Serial.println("Hz");

  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}


void loop() {

  currentTime = millis();
  int averageAcc = 0;
/* Or....get a new sensor event, normalized */ 
  sensors_event_t event; 
  lis.getEvent(&event);
  /* Display the results (acceleration is measured in m/s^2) */
  //xg = event.acceleration.x;
  yg = event.acceleration.y;
  zg = event.acceleration.z;
  //int totAcc = 100* sqrt(yg*yg + zg*zg); //9.3 instead of 9.81 based on acc values
  int totAcc = 100* sqrt(yg*yg); //9.3 instead of 9.81 based on acc values
  averageAcc = averageAcc + totAcc;
  //Serial.println(totAcc);
  delay(5);
  rollingAcc[incrementRolling] = totAcc;      //keeps track of thrown or dropped
  incrementRolling++;
  if(incrementRolling == sizeRolling){
    incrementRolling = 0;
  }
  findAverage();
  sumIncrement++;
  //Serial.println(addFiveAccIncrement);
  if(sumIncrement == 30){
    sumIncrement = 0;
    if(lastAverageAcc > xLastAverageAcc && goingUpFlag == 0){
      goingUpFlag = 1;
      Serial.print("low minus low: ");
      Serial.println(currentTime - lastTimeLow);      
      lastTimeLow = currentTime;
    }
    if(lastAverageAcc < xLastAverageAcc && goingUpFlag == 1){
      goingUpFlag = 0;
      Serial.print("high minus high: ");
      Serial.println(currentTime - lastTimeHigh);
      lastTimeHigh = currentTime;
    }
    xLastAverageAcc = lastAverageAcc;
  }
}

void findAverage(){
  int averageAcc = 0;
  for(int i=0; i<sizeRolling; i++){
    averageAcc = rollingAcc[i] + averageAcc;
  }
  lastAverageAcc = averageAcc/sizeRolling;
  //Serial.println(averageAcc);

  int intensity = map(averageAcc,0,40,0,255);
  colorWipe(strip.Color(intensity, 0, 0), 0); // Red

}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}
