/*
 * AnalogInput.cpp
 *
 *  Created on: Nov 24, 2025
 *      Author: maurizio
 */

#include "DigitalOutput.h"

DigitalOutput::DigitalOutput(string id,string name, Persistance &p, int idx , bool _default,  Hal &hal):
	Base(TYPE_BINARY_OUTPUT,id,name,p),
	hal(hal)
{

	addBoolProperty(PROP_VALUE,Property::MODE_READONLY,_default,"on","off");
	addBoolProperty(PROP_OVERRIDE_VALUE,Property::MODE_WRITABLE_PERSISTENT, _default,"on","off",
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

DigitalOutput::~DigitalOutput() {
}


void DigitalOutput::setValue(bool val)
{
	bool _found;
	bool _override = getPropertyValue<bool>(PROP_OVERRIDE,_found);
	if (_override)
	{
		val = getPropertyValue<bool>(PROP_OVERRIDE_VALUE,_found);
	}
	setPropertyValue<bool>(PROP_VALUE,val);
	hal.setOutput(val);
}

bool DigitalOutput::getValue()
{
	bool _found;
	return getPropertyValue<bool>(PROP_VALUE,_found);
}
