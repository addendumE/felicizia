/*
 * AnalogInput.cpp
 *
 *  Created on: Nov 24, 2025
 *      Author: maurizio
 */

#include "AnalogInput.h"
#include "esp_log.h"

static const char *TAG="AI";
AnalogInput::AnalogInput(string id,string name,Persistance &p, Unit unit, int decimals, float _default):
	Base(TYPE_ANALOG_INPUT,id,name,p)
{

	addFloatProperty(PROP_VALUE,Property::MODE_READONLY,_default,unit,decimals);
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
}

AnalogInput::~AnalogInput() {
}


void AnalogInput::setValue(float val,bool fail)
{
	bool _found;
	bool _override = getPropertyValue<bool>(PROP_OVERRIDE,_found);
	if (_override)
	{
		val = getPropertyValue<float>(PROP_OVERRIDE_VALUE,_found);
		setPropertyValue<bool>(PROP_FAIL,false);
	}
	else
	{
		setPropertyValue<bool>(PROP_FAIL,fail);
	}
	//ESP_LOGI(TAG,"set %f [%d]",val,fail);
	setPropertyValue<float>(PROP_VALUE,val);
}

float AnalogInput::getValue()
{
	bool _found;
	return getPropertyValue<float>(PROP_VALUE,_found);
}
