#ifndef COCOON_LEDS_H
#define COCOON_LEDS_H

/*
 * Animation plan:
 *
 * - Colors emanate from points near the center of each LED strip.
 * - Emanators can move around.
 * - Each of the 4 strips shows the same-ish pattern, but with slight variations.
 * - Colors mutate over time, changing hue/saturation/value at different rates/speeds.
 */

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "Color.h"

#define LED_STRIPS_PIN      (6)
#define LEDS_PER_STRIP      (18)
#define PIXEL_COUNT         (LEDS_PER_STRIP * 4)
#define MAX_BRIGHT          (8)

Adafruit_NeoPixel strip;
uint32_t color;
float wave_phase = 0;
float center_phase = 0;

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
	strip = Adafruit_NeoPixel(PIXEL_COUNT, LED_STRIPS_PIN, NEO_GRB + NEO_KHZ800);
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

	// Update the pixels
	wave_phase += 0.0031f;
	if (wave_phase >= 1.0f) wave_phase -= 1.0f;

	center_phase += 0.0018f;
	if (center_phase >= 1.0f) center_phase -= 1.0f;

	float center_loc = sin01(center_phase * (float)(2*M_PI));
	center_loc = lerp(0.2f, 0.8f, center_loc) * LEDS_PER_STRIP;

	for (uint8_t i = 0; i < LEDS_PER_STRIP; i++) {
		float distanceFromCenter = abs(center_loc - i);

		float wave = sin01(wave_phase * (float)(2*M_PI) - distanceFromCenter);
		wave *= wave;	// somewhat darker

		//wave = max(0, 1.0f - distanceFromCenter);	// testing center location

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
