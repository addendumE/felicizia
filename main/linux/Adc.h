#pragma once
#include <Lock.h>

class Adc: public Lock {
public:
	Adc();
	virtual ~Adc();
	bool init (int ch, int atten) {return true;}
	float readVoltage(int ch);
};
