#ifndef COCOON_LEDS_H
#define COCOON_LEDS_H

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#define LED_STRIP_PIN             (6)
#define LEDS_PER_STRIP      (18)
#define PIXEL_COUNT         (LEDS_PER_STRIP * 4)
#define MAX_BRIGHT          (8)

Adafruit_NeoPixel strip;
uint32_t color;
float phase;

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t cocoon_leds_wheel(byte WheelPos) {
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

void cocoon_leds_init(){
	strip = Adafruit_NeoPixel(PIXEL_COUNT, LED_STRIP_PIN, NEO_GRB + NEO_KHZ800);
	strip.begin();
	strip.show(); // Initialize all pixels to 'off'
}

void cocoon_leds_start_new_color() {
	// Choose a new random color
	color = cocoon_leds_wheel(random(0,255));
}

void cocoon_leds_update() {
	uint16_t red = (color & 0xff0000) >> 16;
	uint16_t green = (color & 0x00ff00) >> 8;
	uint16_t blue = (color & 0x0000ff);

	// Debug: Print R,G,B
	Serial.print(red);
	Serial.print(" ");
	Serial.print(green);
	Serial.print(" ");
	Serial.println(blue);

	// Update the pixels
	phase += 0.02;
	for (uint8_t i = 0; i < LEDS_PER_STRIP; i++) {
		float wave = (sin(i * 0.5f + phase) + 1.0f) * 0.5f;	// 0..1
		uint16_t brt = MAX_BRIGHT * wave;

		uint32_t r = (red * brt) >> 8;
		uint32_t g = (green * brt) >> 8;
		uint32_t b = (blue * brt) >> 8;
		uint32_t c = (r << 16) | (g << 8) | b;

		// Set the same colors all around
		uint8_t p = i;
		if (p < PIXEL_COUNT) strip.setPixelColor(p, c);
		p = LEDS_PER_STRIP * 2 - i - 1;
		if (p < PIXEL_COUNT) strip.setPixelColor(p, c);
		p = LEDS_PER_STRIP * 2 + i;
		if (p < PIXEL_COUNT) strip.setPixelColor(p, c);
		p = LEDS_PER_STRIP * 4 - i - 1;
		if (p < PIXEL_COUNT) strip.setPixelColor(p, c);
	}

	strip.show();
}

#endif
