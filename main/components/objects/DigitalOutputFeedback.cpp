/*
 * AnalogInput.cpp
 *
 *  Created on: Nov 24, 2025
 *      Author: maurizio
 */

#include "DigitalOutputFeedback.h"

DigitalOutputFeedback::DigitalOutputFeedback(string id,string name, Persistance &p , bool _default):
	DigitalOutput(id,name,p,_default),
	oldVal(false),
	fail(false)
{
	addFloatProperty(PROP_FILTER,Property::MODE_WRITABLE_PERSISTENT,10.0,UNIT_SECONDS,0);
	addBoolProperty(PROP_FAIL,Property::MODE_READONLY,false,"yes","no");
	addBoolProperty(PROP_FEEDBACK,Property::MODE_WRITABLE,false,"yes","no");

}

DigitalOutputFeedback::~DigitalOutputFeedback() {
}


void DigitalOutputFeedback::setValue(bool val)
{
	
	if (val && !oldVal)
	{
		tOn=time(NULL);
		fail = false;
	}
	else if (!val)
	{
		fail = false;
	}
	oldVal = val;

	time_t delta = time(NULL) - tOn;
	int flt = (int)getPropertyValueInt(PROP_FILTER);
	bool feed = getPropertyValueBool(PROP_FEEDBACK);

	if (delta > flt && val && !feed)
	{
		fail = true;
	}
	if (fail)
	{
		val = false;
	}
	DigitalOutput::setValue(val);
	setPropertyValueBool(PROP_FAIL,fail);
	
}

void DigitalOutputFeedback::setFeedbackValue(bool val)
{
	setPropertyValueBool(PROP_FEEDBACK,val);
}

