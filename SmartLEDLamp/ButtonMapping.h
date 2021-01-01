#ifndef BUTTONMAPPING_H_
#define BUTTONMAPPING_H_

#include "defines.h"
#include <Arduino.h>

#define RED_FUNCTION 1
#define GREEN_FUNCTION 2
#define BLUE_FUNCTION 3
#define SPEED_FUNCTION 4
#define SHIFT_FUNCTION 5

class ButtonMapping {

public:
	ButtonMapping(int8_t identifier, char const *functionName) :
			identifier(identifier), functionName(functionName) {
	}

	String getButtonIdentifier() {
		switch (identifier) {
		case RED_FUNCTION:
			return "function1";

		case GREEN_FUNCTION:
			return "function2";

		case BLUE_FUNCTION:
			return "function3";

		case SPEED_FUNCTION:
			return "function4";

		case SHIFT_FUNCTION:
			return "function5";

		default:
			return "not defined!";
		}
	}

	const int8_t identifier;
	char const *functionName;
};

#endif /* BUTTONMAPPING_H_ */
