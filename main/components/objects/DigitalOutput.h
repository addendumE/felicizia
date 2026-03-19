#pragma once
#include "Base.h"
#include "Hal.h"
class DigitalOutput:public Base {
public:
	DigitalOutput(string id, string name, Persistance & p, int index , bool _default);
	virtual ~DigitalOutput();
	void setValue(bool val);
	bool getValue();
private:
};
