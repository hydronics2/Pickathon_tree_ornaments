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

#define LED_STRIPS_PIN        (6)
#define LEDS_PER_STRIP        (18)
#define PIXEL_COUNT           (LEDS_PER_STRIP * 4)
#define MAX_BRIGHT            (0x7f)
#define DO_SPIRAL_LIGHTS      (false)

#define PALETTE_STEPS         (10)
#define BAND_SPREAD           (0.23f)

#define MASTER_SPEED          (1.0f)
#define EMIT_SPEED            (0.0151f * MASTER_SPEED)
#define CENTER_DRIFT_SPEED    (0)
#define BRIGHT_MICROS         (8 * 1000)

// When the wind blows: illuminate at about 50%?
#define WIND_MIN              (250 >> 1)
#define WIND_MAX              (1500 >> 1)
#define WIND_BRIGHTNESS_F     (0.5f)

#define IDLE_BRIGHT_TIME_MIN  (0.0005f)
#define IDLE_BRIGHT_TIME_MAX  (0.001f)
#define IDLE_BRIGHTNESS_F     (0.23f)

#define BLACKOUT_MICROS       (3 * 1000000)

Adafruit_NeoPixel strip;

HSV palette[PALETTE_STEPS];

float paletteTween[PALETTE_STEPS];
HSV paletteDest[PALETTE_STEPS];
float paletteTweenDelay[PALETTE_STEPS];

const float CENTER_STAGGER = 0.05f * (0.22f / BAND_SPREAD);
float center_phases[4] = {CENTER_STAGGER * 0, CENTER_STAGGER * 1, CENTER_STAGGER * 2, CENTER_STAGGER * 3};

const float EMIT_STAGGER = 0.1f;
float emit_phases[4] = {EMIT_STAGGER * 1, EMIT_STAGGER * 3, EMIT_STAGGER * 2, EMIT_STAGGER * 0};	// Counts down [1..0] then wraps around

unsigned long lastMicros = 0;

int8_t bandsUntilChangeValue = 0;

// Battery savings: Should be dim/off most of the time.
// During an on-cycle: illuminate at about 25%.
// When the wind blows: illuminate at about 50%?
// Human interaction:
float brightIdle = 1.0f;
float brightIdleAdder = 0.0f;
float brightWind = 0.0f;
float windAmt = 0.0f;

float bright = 1.0f;
float lowBright = 0.0f;
int32_t brightMicrosRemaining = 0;	// update brightness when this flips below zero

int32_t blackoutDelay = 0;
int32_t blackoutMicrosRemaining = 0;

void cocoon_leds_init(){
	for (int8_t i = 0; i < PALETTE_STEPS; i++) {
		paletteTween[i] = 0;
		paletteTweenDelay[i] = 0;
	}

	strip = Adafruit_NeoPixel(PIXEL_COUNT, LED_STRIPS_PIN, NEO_GRB + NEO_KHZ800);
	strip.begin();
	strip.show(); // Initialize all pixels to 'off'
}

void cocoon_do_color_tween(uint32_t hsv32, int delayMicros) {
	if (blackoutMicrosRemaining > 0) return;	// Blackout? Do nothing.

	HSV hsv = (HSV){
		((hsv32 & 0xff0000) >> 16) / 255.0f,
		((hsv32 & 0x00ff00) >>  8) / 255.0f,
		((hsv32 & 0x0000ff)      ) / 255.0f
	};

	int delayMult = lerp(300, 500, randf()) * 1000;

	for (int8_t p = 0; p < PALETTE_STEPS; p++) {
		paletteTween[p] = 1.0f;
		paletteTweenDelay[p] = p * delayMult + delayMicros;

		// Inject a little randomness to the palette
		hsv.h += (randf() - 0.5f) * 0.2f;
		hsv.s += (randf() - 0.5f) * 0.2f;
		hsv.s = clamp(hsv.s, 0.0f, 1.0f);
		hsv.v += (randf() - 0.5f) * 0.3f;
		hsv.v = clamp(hsv.v, 0.5f, 1.0f);
		paletteDest[p] = hsv;
	}

	// We're tweening fully bright, so immediately go dark next
	bandsUntilChangeValue = 1;
}

void cocoon_leds_start_new_color() {
	if (blackoutMicrosRemaining > 0) return;	// Blackout? Do nothing.

	// Choose a random hue
	HSV hsv = palette[0];
	hsv.h += randf() - 0.5f;
	hsv.s = lerp(0.5f, 1.0f, randf());
	hsv.v = lerp(0.6f, 1.0f, randf());
}

uint32_t cocoon_get_current_color() {
	if (blackoutMicrosRemaining > 0) return 0x0;

	HSV hsv = palette[0];

	float hue = hsv.h - floor(hsv.h);

	return (
		(((uint32_t)(hue   * 0xff)) << 16) |
		(((uint32_t)(hsv.s * 0xff)) <<  8) |
		(((uint32_t)(hsv.v * 0xff)))
	);
}

void cocoon_do_blackout(int delayMicros) {
	blackoutDelay = delayMicros;
	blackoutMicrosRemaining = BLACKOUT_MICROS;
}

void cocoon_leds_update(int lastAverageAcc) {
	unsigned long now = micros();
	uint16_t elapsed = (uint16_t)constrain(now - lastMicros, 0, 0xffff);
	lastMicros = now;
	float timeMult = elapsed * (1.0f / 16666.667f);	// Animations are cooked at 60fps

	brightMicrosRemaining -= elapsed;

	while (brightMicrosRemaining <= 0) {
		brightMicrosRemaining += BRIGHT_MICROS;

		// Update idle brightness
		if (brightIdle >= 1.0f) {
			brightIdleAdder = -lerp(IDLE_BRIGHT_TIME_MIN, IDLE_BRIGHT_TIME_MAX, randf());
		} else if (brightIdle <= -2.0f) {
			brightIdleAdder = lerp(IDLE_BRIGHT_TIME_MIN, IDLE_BRIGHT_TIME_MAX, randf());
		}
		brightIdle += brightIdleAdder;

		// Update brightness target
		windAmt = (float)(lastAverageAcc - WIND_MIN) / (WIND_MAX - WIND_MIN);
		windAmt = clamp(windAmt, 0.0f, 1.0f);
		brightWind = windAmt * WIND_BRIGHTNESS_F;

		float brightTarget = max(brightWind, brightIdle * IDLE_BRIGHTNESS_F);

		// Blackout
		if (blackoutDelay > 0) {
			blackoutDelay -= elapsed;

		} else if (blackoutMicrosRemaining > 0) {
			brightTarget = 0;
			blackoutMicrosRemaining -= elapsed;
		}

		float brightSpeed = 0.023f * ((blackoutMicrosRemaining > 0) ? 10 : 1);
		bright = lerp(bright, brightTarget, brightSpeed);
		lowBright = 0.0f;//max(0.0f, bright - 0.75f);

		// Apply color tween(s), if any.
		for (int8_t p = 0; p < PALETTE_STEPS; p++) {
			float prog = (float)p / PALETTE_STEPS;
			float ratio = lerp(0.02, 0.0001, prog);

			if (paletteTweenDelay[p] > 0) {
				paletteTweenDelay[p] -= elapsed;

			} else if (paletteTween[p] >= ratio) {
				paletteTween[p] -= ratio;

				palette[p].h = lerp(paletteDest[p].h, palette[p].h, paletteTween[p]);
				palette[p].s = lerp(paletteDest[p].s, palette[p].s, paletteTween[p]);
				palette[p].v = lerp(paletteDest[p].v, palette[p].v, paletteTween[p]);
			}
		}
	}

	for (uint8_t side = 0; side < 4; side++) {
		// Palette emits out from center.
		float target = emit_phases[side] -= EMIT_SPEED * timeMult * lerp(1.0f, 3.0f, windAmt);

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
			if (DO_SPIRAL_LIGHTS) {
				if (((side + i) % 4) != 0) continue;	// light every 4th LED
			} else {
				if (side != 0) break;	// only light one strip
			}

			float distanceFromCenter = (float)center_loc - (float)i;
			if (distanceFromCenter < 0.0f) distanceFromCenter *= -1.0f;	// like absf()

			// For this distance from the center: Get the two adjacent colors
			float dist = (distanceFromCenter * BAND_SPREAD) + emit_phases[side];
			int8_t c0 = constrain(floorf(dist), 0, PALETTE_STEPS - 1);
			int8_t c1 = constrain(ceilf(dist), 0, PALETTE_STEPS - 1);

			float decimal = dist - c0;
			HSV hsv = lerpHSV(palette[c0], palette[c1], decimal);
			hsv.v *= hsv.v;

			// Apply brightness
			hsv.v = lerp(lowBright, bright, hsv.v);

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
