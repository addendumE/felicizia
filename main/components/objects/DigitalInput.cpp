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
				bool found;
				bool _override = getPropertyValue<bool>(PROP_OVERRIDE,found);
				if (_override)
				{
					setPropertyValue<bool>(PROP_VALUE,ov);
				}
				return Property::SET_OK;
			});
	addBoolProperty(PROP_OVERRIDE,Property::MODE_WRITABLE_PERSISTENT,false,"yes","no",
			[=,this](bool ov) {
				if (ov)
				{
					bool found;
					bool val = getPropertyValue<bool>(PROP_OVERRIDE_VALUE,found);
					setPropertyValue<bool>(PROP_VALUE,val);
				}
				return Property::SET_OK;
			}
	);
}

DigitalInput::~DigitalInput() {
}


void DigitalInput::setValue(bool val)
{
	bool _found;
	bool _override = getPropertyValue<bool>(PROP_OVERRIDE,_found);
	if (_override)
	{
		val = getPropertyValue<bool>(PROP_OVERRIDE_VALUE,_found);
		setPropertyValue<bool>(PROP_VALUE,val);
	}
	else
	{
		if (val == oldValue)
		{
			setPropertyValue<bool>(PROP_VALUE,val);
		}
		oldValue=val;
	}
	
}

bool DigitalInput::getValue()
{
	bool _found;
	return getPropertyValue<bool>(PROP_VALUE,_found);
}
