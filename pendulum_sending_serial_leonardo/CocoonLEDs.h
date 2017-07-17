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
#define MAX_BRIGHT          (0x7f)

#define PALETTE_STEPS       (10)
#define COLOR_SPREAD        (0.15f)

#define MASTER_SPEED        (1.0f)
#define EMIT_SPEED          (0.0131f * MASTER_SPEED)
#define CENTER_DRIFT_SPEED  (0.0007f * MASTER_SPEED)

Adafruit_NeoPixel strip;

float center_phase = 0;

HSV palette[PALETTE_STEPS];
float emit_phase = 0.0f;	// Counts down [1..0] then wraps around

unsigned long lastMicros = 0;

void cocoon_leds_init(){
	strip = Adafruit_NeoPixel(PIXEL_COUNT, LED_STRIPS_PIN, NEO_GRB + NEO_KHZ800);
	strip.begin();
	strip.show(); // Initialize all pixels to 'off'
}

void cocoon_leds_start_new_color() {
	// Choose a random hue
	float hueShift = randf() - 0.5f;
	for (uint8_t i = 0; i < PALETTE_STEPS; i++) {
		palette[i].h += hueShift;
	}
}

void cocoon_leds_update() {
	unsigned long now = micros();
	uint16_t elapsed = (uint16_t)constrain(now - lastMicros, 0, 0xffff);
	lastMicros = now;
	float timeMult = elapsed * (1.0f / 16666.667f);	// Animations are cooked at 60fps

	// Palette emits out from center.
	emit_phase -= EMIT_SPEED * timeMult;
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

	center_phase += CENTER_DRIFT_SPEED * timeMult;
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
		float dist = (distanceFromCenter * COLOR_SPREAD) + emit_phase;
		int8_t c0 = constrain(floorf(dist), 0, PALETTE_STEPS - 1);
		int8_t c1 = constrain(ceilf(dist), 0, PALETTE_STEPS - 1);

		float decimal = dist - c0;
		HSV hsv = lerpHSV(palette[c0], palette[c1], decimal);
		hsv.v *= hsv.v;

		uint32_t color = rgbToUint32(
			hsv2rgb(
				hsv.h,
				hsv.s,
				hsv.v * (MAX_BRIGHT / (float)0xff)
			)
		);

		// Set the same colors all around
		uint8_t p = i;
		if (p < PIXEL_COUNT) strip.setPixelColor(p, color);
		p = LEDS_PER_STRIP * 2 - i - 1;
		if (p < PIXEL_COUNT) strip.setPixelColor(p, color);
		p = LEDS_PER_STRIP * 2 + i;
		if (p < PIXEL_COUNT) strip.setPixelColor(p, color);
		p = LEDS_PER_STRIP * 4 - i - 1;
		if (p < PIXEL_COUNT) strip.setPixelColor(p, color);
	}

	strip.show();
}

#endif
