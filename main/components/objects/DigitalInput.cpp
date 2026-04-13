/*
 * AnalogInput.cpp
 *
 *  Created on: Nov 24, 2025
 *      Author: maurizio
 */

#include "DigitalInput.h"

DigitalInput::DigitalInput(string id,string name, Persistance &p):
	Base(TYPE_BINARY_INPUT,id,name,p),
	oldValue(false)
{

	addBoolProperty(PROP_VALUE,Property::MODE_READONLY,false,"on","off");
	addBoolProperty(PROP_OVERRIDE_VALUE,Property::MODE_WRITABLE_PERSISTENT, false,"on","off",
			[&](bool ov) {
				bool _override = getPropertyValueBool(PROP_OVERRIDE);
				if (_override)
				{
					setPropertyValueBool(PROP_VALUE,ov);
				}
				return Property::SET_OK;
			});
	addBoolProperty(PROP_OVERRIDE,Property::MODE_WRITABLE_PERSISTENT,false,"yes","no",
			[=,this](bool ov) {
				if (ov)
				{
					bool val = getPropertyValueBool(PROP_OVERRIDE_VALUE);
					setPropertyValueBool(PROP_VALUE,val);
				}
				return Property::SET_OK;
			}
	);
}

DigitalInput::~DigitalInput() {
}


void DigitalInput::setValue(bool val)
{
	bool _override = getPropertyValueBool(PROP_OVERRIDE);
	if (_override)
	{
		val = getPropertyValueBool(PROP_OVERRIDE_VALUE);
		setPropertyValueBool(PROP_VALUE,val);
	}
	else
	{
		if (val == oldValue)
		{
			setPropertyValueBool(PROP_VALUE,val);
		}
		oldValue=val;
	}
	
}

bool DigitalInput::getValue()
{
	return getPropertyValueBool(PROP_VALUE);
}
