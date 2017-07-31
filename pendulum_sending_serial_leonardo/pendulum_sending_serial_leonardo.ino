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
#include "CocoonLEDs.h"

#define COCOON               (9)
#define TAP_THRESHOLD        (1000)
#define BLACKOUT_THRESHOLD   (2000)
#define DEV_BLINK            (false)

#define SEND_THROTTLE_MILLIS (500)

#define TAP_DELAY_MICROS     (100 * 1000)

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

uint8_t blink = 0;
const uint8_t BUILTIN_LED_PIN = 17;

int serialChar = 0;
int serialInput[31];
boolean stringComplete = false;

// Users may tilt a cocoon, which would normally blast packets at the router.
// Solution: Throttle the packet rate, and latch the hit-state, so after a
// cocoon is hit, it must return to a resting position before it can send
// another packet.
unsigned long lastSentMillis = 0;
bool isLatchOn = false;

void setup(void) {
  pinMode(BUILTIN_LED_PIN, OUTPUT);

  // Generate numbers with randomness
  randomSeed(analogRead(0) << 10 | analogRead(1));

  Serial.begin(115200);
  Serial1.begin(115200);

  cocoon_leds_init();

  Serial.println("LIS3DH test!");
  if (! lis.begin(0x18)) {   // change this to 0x19 for alternative i2c address
    Serial.println("Couldnt start");
    while (1);
  }
  Serial.println("LIS3DH found!");

  lis.setRange(LIS3DH_RANGE_4_G);   // 2, 4, 8 or 16 G!

  //1, 10, 25, 50, 100, 200, 400_HZ, LOWPOWER_1K6HZ, _5KHZ
  lis.setDataRate(LIS3DH_DATARATE_400_HZ);

  Serial.print("Range = "); Serial.print(2 << lis.getRange());
  Serial.println("G");

  Serial.print("Data rate = "); Serial.print(lis.getDataRate());
  Serial.println("Hz");

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
  //Serial.print("yNeutral: ");
  //Serial.println(yNeutral);
  //Serial.print("zNeutral: ");
  zNeutral = z2/20;
  //Serial.println(zNeutral);
}


void loop() {
  // Blink to prove that we're alive
  if (!DEV_BLINK) blink = 1;
  digitalWrite(BUILTIN_LED_PIN, (blink == 0) ? LOW : HIGH);
  blink = (blink + 1) & 0x3f;

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

  bool isThrottled = (currentTime - lastSentMillis) <= SEND_THROTTLE_MILLIS;

  if(lastAverageAcc > TAP_THRESHOLD){ //TAP detected send it to ESP

    // Hard hit? Do a blackout
    if (lastAverageAcc > BLACKOUT_THRESHOLD) {
      cocoon_do_blackout(0);

      memset(rollingAcc, 0, sizeof(rollingAcc)); //clear array so it doesn't take forever to normalize

      return;
    }

    cocoon_leds_start_new_color();
    uint32_t color = cocoon_get_current_color();
    cocoon_do_color_tween(color, 0);

    if (!isThrottled && !isLatchOn) {
      sendTapData(color);
      lastSentMillis = currentTime;
    }

    memset(rollingAcc, 0, sizeof(rollingAcc)); //clear array so it doesn't take forever to normalize

    isLatchOn = true;

    return;

  } else {
		// Back to a resting position. If we're not throttled: Disable the latch.
		if (!isThrottled) {
			isLatchOn = false;
		}
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

  cocoon_leds_update(lastAverageAcc);
}

void findAverage(){
  long averageAcc = 0;
  for(int i=0; i<sizeRolling; i++){
    averageAcc = rollingAcc[i] + averageAcc;
  }
  lastAverageAcc = averageAcc/sizeRolling;
  //Serial.println(lastAverageAcc);
}

uint8_t cleanByte(uint8_t b)
{
	// '[' is our packet header, we'll fudge this a bit: Don't send that value.
	if (b == '[') b++;
	if (b == ']') b++;
	return b;
}

// sends data to ESP at 115200
// sending Coccoon#, Red,Green,Blue, Intensity
void sendTapData(uint32_t currentColor){
  Serial1.write('['); //start value
  Serial.println('['); //start value

  Serial1.write(COCOON);
  Serial.println(COCOON);

  uint8_t red = ((currentColor & 0xff0000) >> 16);
  uint8_t green = ((currentColor & 0x00ff00) >>  8);
  uint8_t blue = ((currentColor & 0x0000ff)      );

	red = cleanByte(red);
	green = cleanByte(green);
	blue = cleanByte(blue);

  Serial1.write(red); //red
  Serial.println(red); //red

  Serial1.write(green); //green
  Serial.println(green); //green

  Serial1.write(blue); //blue
  Serial.println(blue); //blue

  char tapValue = map(lastAverageAcc, 2000,6000,0,255);
	tapValue = cleanByte(tapValue);
  Serial1.write(tapValue); //tap value
  Serial.println(tapValue); //tap value

  Serial1.write(']');  //end value
  Serial.println(']');  //end value
  Serial.println("");
}

//--------------------------------------------------------- SERIAL EVENT - UDP Broadcast from Server
//Example Data: 91,123,228,43,13,25,24,22,21,19,18,16,15,14,12,11,9,8,7,5,4,2,1,0,1,
// 91 is indicates the beginning of the data, '['
//Next Three Bytes are RGB - 123,228,43
//Next Byte is Intensity - 13 (0-254)
//Next Byte is global brightness (0-63)
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

    }else if(serialChar < 30){
      Serial.println(inChar);
      serialInput[serialChar] = inChar;

			if (serialChar == 4) {
				Serial.print(">>> Brightness: ");
				Serial.println(inChar * 1);

				cocoon_set_brightness(inChar);

			// If this byte corresponds to this COCOON number: Process the hit,
			// and do a color flash.
			} else if (serialChar == (COCOON + 5)) {

				Serial.print("*** Flash! ");
				Serial.println(inChar * 1);

				// Exception: If the delay is 0, then this cocoon triggered the tap, and
				// we already did the color change.
				if (inChar != 0) {
					uint32_t color = (serialInput[0] << 16) | (serialInput[1] << 8) | serialInput[2];
					long delay = inChar * TAP_DELAY_MICROS;
					cocoon_do_color_tween(color, delay);
				}
			}

      serialChar++;
    }
  }
}
