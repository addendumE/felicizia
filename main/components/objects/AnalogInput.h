#pragma once
#include "Base.h"

class AnalogInput: public Base {
public:
	AnalogInput(string id, string name, Persistance &p, Unit unit = UNIT_NONE , int decimals = 0, float _default = 0.0f);
	virtual ~AnalogInput();
	void setValue(float val, bool fail = false);
	float getValue();
private:
};