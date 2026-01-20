/*
 * AnalogInput.cpp
 *
 *  Created on: Nov 24, 2025
 *      Author: maurizio
 */

#include "AnalogInputCal.h"
AnalogInputCal::AnalogInputCal(string id,string name,Persistance &p, Unit unit, int decimals, float _default):
	AnalogInput(id,name,p, unit, decimals, _default)
{
	addFloatProperty(PROP_CAL_OFFSET,Property::MODE_WRITABLE_PERSISTENT,0.0f,unit,decimals);
	addFloatProperty(PROP_CAL_GAIN,Property::MODE_WRITABLE_PERSISTENT,1.0f,UNIT_NONE,decimals);
}

AnalogInputCal::~AnalogInputCal() {
}


void AnalogInputCal::setValue(float val,bool fail)
{

	bool _found;
	float cal_gain =  getPropertyValue<float>(PROP_CAL_GAIN,_found);
	float cal_offset = getPropertyValue<float>(PROP_CAL_OFFSET,_found);
	val = cal_offset + (val * cal_gain);
	AnalogInput::setValue(val,fail);
}

float AnalogInputCal::getValue()
{
	return AnalogInput::getValue();
}
