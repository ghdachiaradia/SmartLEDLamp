#ifndef GLOWVISUALIZER_H_
#define GLOWVISUALIZER_H_

#include "Visualizer.h"
#include "FastLED.h"

class GlowVisualizer: public Visualizer {
public:
	GlowVisualizer();
	virtual ~GlowVisualizer();

	virtual void start();
	virtual void stop();
	virtual void setColor(int color1Hsv, int color2Hsv, int color3Hsv);
	virtual boolean onButtonPressed(uint8_t button);

protected:
	virtual void computeImage();

private:
	int8 fade = 5;
	uint8_t brightness = 0;
	uint8_t holdTime = 0;
	bool runUp = false;
	CRGB currentColor;
	void updateFsHeight();
};

#endif /* GLOWVISUALIZER_H_ */
