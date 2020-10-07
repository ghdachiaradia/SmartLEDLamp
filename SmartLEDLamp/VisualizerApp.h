#ifndef VISUALIZERAPP_H_
#define VISUALIZERAPP_H_

#include "App.h"
#include "Visualizer.h"

class VisualizerApp: public App {
public:
	VisualizerApp(LEDMatrix* pLEDMatrix, String name);
	virtual ~VisualizerApp();

	virtual void start();
	virtual void setColor(int color1Hsv, int color2Hsv, int color3Hsv);
	virtual void run();
	virtual void update();
	virtual void stop();

	virtual std::vector<ButtonMapping> getButtonMappings();
	virtual boolean onButtonPressed(uint8_t button);

	void setVisualizer(uint8_t idx, Visualizer* pVisualizer);

private:
	Visualizer* visualizers[2] = { NULL, NULL };
	unsigned long int lastMillis;

	uint8_t* pImageData[2] = { NULL, NULL };
};

#endif /* VISUALIZERAPP_H_ */
