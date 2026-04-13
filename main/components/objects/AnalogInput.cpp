/*
 * AnalogInput.cpp
 *
 *  Created on: Nov 24, 2025
 *      Author: maurizio
 */

#include "AnalogInput.h"
AnalogInput::AnalogInput(string id,string name,Persistance &p, Unit unit, int decimals, float _default):
	Base(TYPE_ANALOG_INPUT,id,name,p)
{
	addFloatProperty(PROP_VALUE,Property::MODE_READONLY,_default,unit,decimals);
	addFloatProperty(PROP_FILTER,Property::MODE_WRITABLE_PERSISTENT,1.0,UNIT_NONE,2);
	addFloatProperty(PROP_OVERRIDE_VALUE,Property::MODE_WRITABLE_PERSISTENT, _default,unit,decimals,
			[&](float ov) {
				bool _override = getPropertyValueBool(PROP_OVERRIDE);
				if (_override)
				{
					setPropertyValueFloat(PROP_VALUE,ov);
				}
				return Property::SET_OK;
			});
	addBoolProperty(PROP_OVERRIDE,Property::MODE_WRITABLE_PERSISTENT,false,"yes","no",
			[=,this](bool ov) {
				if (ov)
				{
					float val = getPropertyValueFloat(PROP_OVERRIDE_VALUE);
					setPropertyValueFloat(PROP_VALUE,val);
				}
				return Property::SET_OK;
			}
	);
	addBoolProperty(PROP_FAIL,Property::MODE_READONLY,false,"yes","no");
	addFloatProperty(PROP_CAL_OFFSET,Property::MODE_WRITABLE_PERSISTENT,0.0f,unit,decimals);
	addFloatProperty(PROP_CAL_GAIN,Property::MODE_WRITABLE_PERSISTENT,1.0f,UNIT_NONE,decimals);
}

AnalogInput::~AnalogInput() {
}


void AnalogInput::setValue(float val,bool fail)
{
	float cal_gain =  getPropertyValueFloat(PROP_CAL_GAIN);
	float cal_offset = getPropertyValueFloat(PROP_CAL_OFFSET);
	val = cal_offset + (val * cal_gain);

	bool _override = getPropertyValueBool(PROP_OVERRIDE);
	if (_override)
	{
		val = getPropertyValueFloat(PROP_OVERRIDE_VALUE);
		setPropertyValueFloat(PROP_FAIL,false);
		setPropertyValueFloat(PROP_VALUE,val);
	}
	else
	{
		setPropertyValueBool(PROP_FAIL,fail);
		float flt =getPropertyValueFloat(PROP_FILTER);
		float old = getPropertyValueFloat(PROP_VALUE);
		if (flt >1.0f ) flt = 1.0f;
		val = val*flt+old*(1.0f-flt);
		setPropertyValueFloat(PROP_VALUE,val);
	}
}

float AnalogInput::getValue()
{
	return getPropertyValueFloat(PROP_VALUE);
}
