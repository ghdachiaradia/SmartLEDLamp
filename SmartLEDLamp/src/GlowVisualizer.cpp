#include "GlowVisualizer.h"
#include "defines.h"
#include "Log.h"

GlowVisualizer::GlowVisualizer() {
	currentColor = CRGB::Red;
}

GlowVisualizer::~GlowVisualizer() {
}

void GlowVisualizer::computeImage() {
	brightness = brightness + fade;

	if ((brightness < 6 && fade < 0) || (brightness > 150 && fade > 0)) {
		fade = -fade;

		int8 random = random8(1, 5);
		if (fade < 0) {
			fade = -random;
		} else {
			fade = random;
		}

		Logger.debug("Fade '%i'/'%i'", fade, brightness);
	}


	CRGB color = CRGB(currentColor);
	color.fadeToBlackBy(brightness);

	for (int x = 0; x < LEDS_HEIGHT; x++) {
		for (int y = 0; y < LEDS_WIDTH; y++) {
			imageData[((x) * LEDS_WIDTH + y) * 3 + 0] = color.r;
			imageData[((x) * LEDS_WIDTH + y) * 3 + 1] = color.g;
			imageData[((x) * LEDS_WIDTH + y) * 3 + 2] = color.b;
		}
	}
}

void GlowVisualizer::start() {
}

void GlowVisualizer::stop() {
}

void GlowVisualizer::setColor(int color1Hsv, int color2Hsv, int color3Hsv) {
	currentColor = CHSV(color1Hsv, 255, 255);
}

boolean GlowVisualizer::onButtonPressed(uint8_t button) {
	boolean handled = false;

	switch (button) {

	case BTN_BLUE_UP:
		handled = true;
		break;
	case BTN_BLUE_DOWN:
		handled = true;
		break;
	}

	return handled;
}
