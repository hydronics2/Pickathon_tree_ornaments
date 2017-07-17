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
#define BAND_SPREAD         (0.22f)

#define MASTER_SPEED        (1.0f)
#define EMIT_SPEED          (0.0151f * MASTER_SPEED)
#define CENTER_DRIFT_SPEED  (0.0007f * MASTER_SPEED)

Adafruit_NeoPixel strip;

HSV palette[PALETTE_STEPS];

const float CENTER_STAGGER = 0.05f * (0.22f / BAND_SPREAD);
float center_phases[4] = {CENTER_STAGGER * 0, CENTER_STAGGER * 1, CENTER_STAGGER * 2, CENTER_STAGGER * 3};

const float EMIT_STAGGER = 0.1f;
float emit_phases[4] = {EMIT_STAGGER * 1, EMIT_STAGGER * 3, EMIT_STAGGER * 2, EMIT_STAGGER * 0};	// Counts down [1..0] then wraps around

unsigned long lastMicros = 0;

int8_t bandsUntilChangeValue = 0;

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

	for (uint8_t side = 0; side < 4; side++) {
		// Palette emits out from center.
		float target = emit_phases[side] -= EMIT_SPEED * timeMult;

		if (emit_phases[side] <= 0.0f) {	// Time to choose a new color?

			// Bump up the emit_phases so the color change looks seamless
			for (uint8_t s = 0; s < 4; s++) {
				emit_phases[s] += 1.0f;
			}

			// Bucket brigade: Shift the colors down
			for (int8_t i = PALETTE_STEPS - 2; i >= 0; i--) {
				palette[i + 1] = palette[i];
			}

			// Tweak palette[0], so a new color emerges.
			uint8_t r = (uint8_t)(random(0, 0xff));
			if (r & 0x1) {	// change hue
				palette[0].h += randf() * 0.1 - 0.05;
			}

			palette[0].s = lerp(0.8f, 1.0f, randf());

			bandsUntilChangeValue--;
			if (bandsUntilChangeValue <= 0) {
				bandsUntilChangeValue = random(1, 4);

				bool isOn = palette[0].v > 0.1f;
				palette[0].v = (isOn ? 0.0f : 1.0f);
			}

			// Rando brightnesses
			if (palette[0].v > 0.1f) {
				palette[0].v = lerp(0.6f, 1.0f, randf());
			}

		}

		center_phases[side] += CENTER_DRIFT_SPEED * timeMult;
		if (center_phases[side] >= 1.0f) center_phases[side] -= 1.0f;

		float center_loc = sin01(center_phases[side] * (float)(2*M_PI));
		center_loc = lerp(0.2f, 0.8f, center_loc) * LEDS_PER_STRIP;

		for (uint8_t i = 0; i < LEDS_PER_STRIP; i++) {
			float distanceFromCenter = (float)center_loc - (float)i;
			if (distanceFromCenter < 0.0f) distanceFromCenter *= -1.0f;	// like absf()

			// For this distance from the center: Get the two adjacent colors
			float dist = (distanceFromCenter * BAND_SPREAD) + emit_phases[side];
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
			if ((side == 0) && (p < PIXEL_COUNT)) strip.setPixelColor(p, color);
			p = LEDS_PER_STRIP * 2 - i - 1;
			if ((side == 1) && (p < PIXEL_COUNT)) strip.setPixelColor(p, color);
			p = LEDS_PER_STRIP * 2 + i;
			if ((side == 2) && (p < PIXEL_COUNT)) strip.setPixelColor(p, color);
			p = LEDS_PER_STRIP * 4 - i - 1;
			if ((side == 3) && (p < PIXEL_COUNT)) strip.setPixelColor(p, color);

		}	// ! each LED

	}	// ! each side

	strip.show();
}

#endif
