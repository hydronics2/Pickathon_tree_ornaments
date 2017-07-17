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
#define MAX_BRIGHT          (0xff)

#define PALETTE_STEPS       (10)

Adafruit_NeoPixel strip;
//uint32_t color;
//float wave_phase = 0;
float center_phase = 0;

HSV palette[PALETTE_STEPS];
float emit_phase = 0.0f;	// Counts down [1..0] then wraps around

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
/*
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
*/

void cocoon_leds_init(){
	strip = Adafruit_NeoPixel(PIXEL_COUNT, LED_STRIPS_PIN, NEO_GRB + NEO_KHZ800);
	strip.begin();
	strip.show(); // Initialize all pixels to 'off'
}

void cocoon_leds_start_new_color() {
	// Choose a new random color
	//color = cocoon_leds_wheel(random(0,255));

	// Choose a random hue
	float hue = randf();
	for (uint8_t i = 0; i < PALETTE_STEPS; i++) {
		palette[i].h = hue;
	}
}

void cocoon_leds_update() {
	// Palette emits out from center.
	emit_phase -= 0.0131f;
	if (emit_phase <= 0.0f) {	// Time to choose a new color?
		emit_phase += 1.0f;

		// Bucket brigade: Shift the colors down
		for (int8_t i = PALETTE_STEPS - 2; i >= 0; i--) {
			palette[i + 1] = palette[i];
		}

		// Tweak palette[0], so a new color emerges.
		uint8_t r = (uint8_t)(random(0, 0xff));
		if (r & 0x1) {	// change hue
			palette[0].h += randf() * 0.1 - 0.05;
		}

		/*
		if (r & 0x2) {	// change saturation
			palette[0].s = randf();
		}
		*/
		palette[0].s = 0.9;

		/*
		if (r & 0x4) {	// change value
			palette[0].v = randf();
		}
		*/
		palette[0].v = 1.0f - palette[0].v;
	}

	// Update the pixels
	/*
	wave_phase += 0.0031f;
	if (wave_phase >= 1.0f) wave_phase -= 1.0f;
	*/

	center_phase += 0.0007f;
	if (center_phase >= 1.0f) center_phase -= 1.0f;

	float center_loc = sin01(center_phase * (float)(2*M_PI));
	center_loc = lerp(0.2f, 0.8f, center_loc) * LEDS_PER_STRIP;

	/*
	uint16_t red = (color & 0xff0000) >> 16;
	uint16_t green = (color & 0x00ff00) >> 8;
	uint16_t blue = (color & 0x0000ff);
	*/

	for (uint8_t i = 0; i < LEDS_PER_STRIP; i++) {
		float distanceFromCenter = (float)center_loc - (float)i;
		if (distanceFromCenter < 0.0f) distanceFromCenter *= -1.0f;	// like absf()

		/*
		float wave = sin01(wave_phase * (float)(2*M_PI) - distanceFromCenter);
		wave *= wave;	// somewhat darker

		//wave = max(0, 1.0f - distanceFromCenter);	// testing center location

		uint16_t brt = MAX_BRIGHT * wave;

		uint32_t r = (red * brt) >> 8;
		uint32_t g = (green * brt) >> 8;
		uint32_t b = (blue * brt) >> 8;
		uint32_t c = (r << 16) | (g << 8) | b;
		*/

		// For this distance from the center: Get the two adjacent colors
		float dist = (distanceFromCenter * 0.4f) + emit_phase;
		int8_t c0 = constrain(floorf(dist), 0, PALETTE_STEPS - 1);
		int8_t c1 = constrain(ceilf(dist), 0, PALETTE_STEPS - 1);

		float decimal = dist - c0;
		HSV hsv = lerpHSV(palette[c0], palette[c1], decimal);

		uint32_t color = rgbToUint32(hsv2rgb(hsv.h, hsv.s, hsv.v));

		// Set the same colors all around
		uint8_t p = i;
		if (p < PIXEL_COUNT) strip.setPixelColor(p, color);
		/*
		p = LEDS_PER_STRIP * 2 - i - 1;
		if (p < PIXEL_COUNT) strip.setPixelColor(p, color);
		p = LEDS_PER_STRIP * 2 + i;
		if (p < PIXEL_COUNT) strip.setPixelColor(p, color);
		p = LEDS_PER_STRIP * 4 - i - 1;
		if (p < PIXEL_COUNT) strip.setPixelColor(p, color);
		*/
	}

	strip.show();
}

#endif
