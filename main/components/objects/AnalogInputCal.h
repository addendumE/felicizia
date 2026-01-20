#pragma once
#include "AnalogInput.h"

class AnalogInputCal: public AnalogInput {
public:
	AnalogInputCal(string id, string name, Persistance &p, Unit unit = UNIT_NONE , int decimals = 0, float _default = 0.0f);
	virtual ~AnalogInputCal();
	void setValue(float val, bool fail = false);
	float getValue();
private:
};
