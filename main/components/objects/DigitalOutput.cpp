/*
 * AnalogInput.cpp
 *
 *  Created on: Nov 24, 2025
 *      Author: maurizio
 */

#include "DigitalOutput.h"

DigitalOutput::DigitalOutput(string id,string name, Persistance &p , bool _default):
	Base(TYPE_BINARY_OUTPUT,id,name,p)
{

	addBoolProperty(PROP_VALUE,Property::MODE_READONLY,_default,"on","off");
	addBoolProperty(PROP_OVERRIDE_VALUE,Property::MODE_WRITABLE_PERSISTENT, _default,"on","off",
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

DigitalOutput::~DigitalOutput() {
}


void DigitalOutput::setValue(bool val)
{
	bool _override = getPropertyValueBool(PROP_OVERRIDE);
	if (_override)
	{
		val = getPropertyValueBool(PROP_OVERRIDE_VALUE);
	}
	setPropertyValueBool(PROP_VALUE,val);
}

bool DigitalOutput::getValue()
{
	bool _found;
	return getPropertyValueBool(PROP_VALUE);
}
