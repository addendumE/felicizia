#pragma once
#include "Base.h"

class Threshold:public Base {
public:
	enum Mode
	{
		MODE_OVER,
		MODE_UNDER,
		MODE_IN,
		MODE_OUT
	};
	Threshold(string id, string name, Persistance &p, Unit unit = UNIT_NONE , int decimals = 0, bool _default=false, Mode m = MODE_OVER);
	virtual ~Threshold();
	bool setValue(float val1);
	bool getValue();
	void getThValue(float &h, float &l);
private:
};
