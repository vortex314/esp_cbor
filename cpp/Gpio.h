/*
 * Gpio.h
 *
 *  Created on: Nov 17, 2015
 *      Author: lieven
 */

#ifndef GPIO_H_
#define GPIO_H_
#include "Erc.h"

class Gpio {
private:
	uint8_t _pin;
	bool _digitalValue;
	bool _input;
	// I/O : D/P
	typedef enum {
		MODE_DISABLED = 0,
		MODE_OUTPUT_OPEN_DRAIN,
		MODE_OUTPUT_PUSH_PULL,
		MODE_INPUT,
		MODE_INPUT_PULLUP,

	} Mode;
	Mode _mode;
	static const char* _sMode[];
public:
	Gpio(uint8_t pin) IROM;
	virtual ~Gpio() IROM;
	Erc setMode(const char* str) IROM;
	Erc getMode(char* str) IROM;
	Erc digitalWrite(uint8_t i) IROM;
	Erc digitalRead(uint8_t* i) IROM;
	Erc analogRead(int v) IROM;
	Erc analogWrite(int* v) IROM;
};

#endif /* GPIO_H_ */
