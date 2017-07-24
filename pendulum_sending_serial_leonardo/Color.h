#ifndef COLOR_H
#define COLOR_H

// Trig functions squished to range 0..1
inline float sin01(float n) {
	return ((float)sin(n) + 1.0f) * 0.5f;
}

inline float cos01(float n) {
	return ((float)cos(n) + 1.0f) * 0.5f;
}

inline float lerp(float a, float b, float p) {
	return a + (b - a) * p;
}

inline float clamp(float x, float _min, float _max) {
	return (x < _min) ? _min : ((x > _max) ? _max : x);
}

inline float randf() {
	return random(0, 0xffffff) * (1.0f / 0xffffff);
}

struct RGB8 {
	uint_fast8_t r;
	uint_fast8_t g;
	uint_fast8_t b;
};

struct RGB {
	float r;
	float g;
	float b;
};

inline RGB lerpRGB(RGB c0, RGB c1, float p) {
	return (RGB){
		lerp(c0.r, c1.r, p),
		lerp(c0.g, c1.g, p),
		lerp(c0.b, c1.b, p)
	};
}

inline uint32_t rgbToUint32(RGB rgb) {
	return ((uint32_t)(rgb.r * 0xff) << 16) |
	       ((uint32_t)(rgb.g * 0xff) << 8)  |
				  (uint32_t)(rgb.b * 0xff);
}

struct HSV {
	float h;
	float s;
	float v;
};

inline HSV lerpHSV(HSV c0, HSV c1, float p) {
	return (HSV){
		lerp(c0.h, c1.h, p),
		lerp(c0.s, c1.s, p),
		lerp(c0.v, c1.v, p)
	};
}

// Convert HSV values to RGB
// All arguments are in range 0..1 . They are not checked for sane values!
// Return values are in range 0..1
// Ported from: http://code.activestate.com/recipes/576554-covert-color-space-from-hsv-to-rgb-and-rgb-to-hsv/
RGB hsv2rgb (float hF, float sF, float vF) {
  // Expand H by 6 times for the 6 color regions. Wrap the remainder to 0..1
  float hWrap = (hF*6.0);
  hWrap = hWrap - floor(hWrap);  // floating point remainder. 0..1 for each of the 6 color regions

  float v = vF;  // top (max) value
  float p = vF * (1.0-sF);  // bottom (min) value
  float q = vF * (1.0-(hWrap*sF));  // falling (yellow->green: the red channel falls)
  float t = vF * (1.0 - ((1.0-hWrap)*sF));  // rising (red->yellow: the green channel rises)

  // Need to find the correct color region that the hue belongs to.
  // Hue can have a negative value, so need to step it to positive space, so the modulus % operator behaves as expected.
  float hF_pos = hF;
  if( hF_pos < 0.0 ) {
    hF_pos += ceil(-hF_pos);
  }
  uint8_t hue_i = (uint8_t)(floor( hF_pos * 6.0 )) % 6;

  switch( hue_i ) {
    case 0:  return (RGB){v,t,p};  // red -> yellow
    case 1:  return (RGB){q,v,p};  // yellow -> green
    case 2:  return (RGB){p,v,t};  // green -> cyan
    case 3:  return (RGB){p,q,v};  // cyan -> blue
    case 4:  return (RGB){t,p,v};  // blue -> magenta
    case 5:  return (RGB){v,p,q};  // magenta -> red
  }

  return (RGB){0,0,0};  // sanity escape
};

/*
HSV rgb2hsv (uint32_t rgb) {
	uint8_t r = (uint8_t)((rgb & 0xff0000) >> 16);
	uint8_t g = (uint8_t)((rgb & 0x00ff00) >> 8);
	uint8_t b = (uint8_t)((rgb & 0x0000ff));

	float inR = r * (1.0f / 0xff);
	float inG = g * (1.0f / 0xff);
	float inB = b * (1.0f / 0xff);

  float outH, outS, outV;
  float delta;

  float _min = min( inR, min( inG, inB ));
  float _max = max( inR, max( inG, inB ));

  delta = _max - _min;

  // Grayscale / black / white?
  if( delta==0.0 ) {
    return (HSV){ _max, _max, _max };
  }

  outS = (delta / _max);               // Saturation
  outV = _max;                           // Value

  if( inR == _max ) {
    outH = ( inG - inB ) / delta;        // between yellow & magenta
  } else if( inG >= _max ) {
    outH = 2.0 + ( inB - inR ) / delta;  // between cyan & yellow
  } else {
    outH = 4.0 + ( inR - inG ) / delta;  // between magenta & cyan
  }

  outH *= (1.0/6.0);  // range: 0..1

  if( outH < 0.0 ) {
    outH += 1.0;
  }

  return (HSV){ outH, outS, outV };
}
*/

#endif
