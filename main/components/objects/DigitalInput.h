#pragma once
#include "Base.h"
class DigitalInput:public Base {
public:
	DigitalInput(string id, string name, Persistance & p);
	virtual ~DigitalInput();
	void setValue(bool val);
	bool getValue();
private:
	bool oldValue;
};
