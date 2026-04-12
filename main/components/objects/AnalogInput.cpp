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
				bool found;
				bool _override = getPropertyValue<bool>(PROP_OVERRIDE,found);
				if (_override)
				{
					setPropertyValue<float>(PROP_VALUE,ov);
				}
				return Property::SET_OK;
			});
	addBoolProperty(PROP_OVERRIDE,Property::MODE_WRITABLE_PERSISTENT,false,"yes","no",
			[=,this](bool ov) {
				if (ov)
				{
					bool found;
					float val = getPropertyValue<float>(PROP_OVERRIDE_VALUE,found);
					setPropertyValue<float>(PROP_VALUE,val);
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

	bool _found;
	float cal_gain =  getPropertyValue<float>(PROP_CAL_GAIN,_found);
	float cal_offset = getPropertyValue<float>(PROP_CAL_OFFSET,_found);
	val = cal_offset + (val * cal_gain);

	bool _override = getPropertyValue<bool>(PROP_OVERRIDE,_found);
	if (_override)
	{
		val = getPropertyValue<float>(PROP_OVERRIDE_VALUE,_found);
		setPropertyValue<bool>(PROP_FAIL,false);
		setPropertyValue<float>(PROP_VALUE,val);
	}
	else
	{
		setPropertyValue<bool>(PROP_FAIL,fail);
		bool _found;
		float flt =getPropertyValue<float>(PROP_FILTER,_found);
		float old = getPropertyValue<float>(PROP_VALUE,_found);
		if (flt >1.0f ) flt = 1.0f;
		val = val*flt+old*(1.0f-flt);
		setPropertyValue<float>(PROP_VALUE,val);
	}
	//ESP_LOGI(TAG,"set %f [%d]",val,fail);
}

float AnalogInput::getValue()
{
	bool _found;
	return getPropertyValue<float>(PROP_VALUE,_found);
}
