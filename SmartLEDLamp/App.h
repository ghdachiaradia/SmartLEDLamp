#ifndef APP_H_
#define APP_H_

#include "LEDMatrix.h"

class App {
public:
	App(LEDMatrix* pLEDMatrix, String name);
	virtual ~App();

	virtual void start() {
	}

	virtual void stop() {
	}

	virtual void setColor(int color1Hsv, int color2Hsv, int color3Hsv) {
	}

	virtual boolean onButtonPressed(uint8_t button) {
		return false;
	}

	virtual void run() = 0;
	virtual void update() = 0;

	const String getName() const {
		return name;
	}

protected:
	String name;
	LEDMatrix *pMatrix;
};

#endif /* APP_H_ */
