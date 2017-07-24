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
	Serial.print("yNeutral: ");
	Serial.println(yNeutral);
	Serial.print("zNeutral: ");
	zNeutral = z2/20;
	Serial.println(zNeutral);
}


void loop() {
	// Blink to prove that we're alive
	if (!DEV_BLINK) blink = 1;
	digitalWrite(BUILTIN_LED_PIN, (blink == 0) ? LOW : HIGH);
	blink = (blink + 1) & 0x3f;

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

	if(lastAverageAcc > TAP_THRESHOLD){ //TAP detected send it to ESP

		// Hard hit? Do a blackout
		if (lastAverageAcc > BLACKOUT_THRESHOLD) {
			cocoon_do_blackout(0);

			memset(rollingAcc, 0, sizeof(rollingAcc)); //clear array so it doesn't take forever to normalize

			return;
		}

		sendTapData();

		cocoon_leds_start_new_color();
		uint32_t color = cocoon_get_current_color();
		cocoon_do_color_tween(color, 0);

		memset(rollingAcc, 0, sizeof(rollingAcc)); //clear array so it doesn't take forever to normalize

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

	cocoon_leds_update(lastAverageAcc);
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
	Serial1.write(COCOON);
	char tapValue = map(lastAverageAcc, 2000,6000,0,255);
	Serial.print("tapValue: ");
	Serial.println(tapValue);
	Serial1.write(tapValue);
	Serial1.write(']');  //end value
}
