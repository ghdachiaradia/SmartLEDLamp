#ifndef CAROUSEL_H_
#define CAROUSEL_H_

#include "Visualizer.h"

class Carousel: public Visualizer {
public:
	Carousel(uint8_t _fsSpeed, uint8_t _fsZoom);
	virtual ~Carousel();

	virtual void start();
	virtual void stop();
	virtual std::vector<ButtonMapping> getButtonMappings();

	virtual boolean onButtonPressed(uint8_t button);

protected:
	virtual void computeImage();

private:
	int8_t fsSpeed;
	int16_t fsZoom;
	uint16_t fsIndex;
	int16_t* pFsHeight;
	uint8_t* pFsLut;

	void updateFsHeight();
};

#endif /* CAROUSEL_H_ */
