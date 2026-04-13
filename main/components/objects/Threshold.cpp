/*
 * AnalogInput.cpp
 *
 *  Created on: Nov 24, 2025
 *      Author: maurizio
 */

#include "Threshold.h"
#include "esp_log.h"
static  const char* TAG="TH";

Threshold::Threshold(string id,string name,Persistance &p, Unit unit, int decimals, bool _default, Mode m):
	Base(TYPE_THRESHOLD,id,name,p)
{

	addBoolProperty(PROP_VALUE,Property::MODE_READONLY,_default,"active","inactive");
	addBoolProperty(PROP_OVERRIDE_VALUE,Property::MODE_WRITABLE_PERSISTENT, _default,"active","inactive",
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
	addFloatProperty(PROP_TH_L,Property::MODE_WRITABLE_PERSISTENT,0.0f,unit,decimals);
	addFloatProperty(PROP_TH_H,Property::MODE_WRITABLE_PERSISTENT,0.0f,unit,decimals);
	addFloatProperty(PROP_TH_LAST_VALUE,Property::MODE_READONLY,0.0f,unit,decimals);

	vector <string> strings;
	strings.push_back("over");
	strings.push_back("under");
	strings.push_back("inside");
	strings.push_back("outside");
	addEnumProperty(PROP_TH_MODE,Property::MODE_READONLY,m,&strings);
}

Threshold::~Threshold() {
}


bool Threshold::setValue(float measure)
{
	bool _override = getPropertyValueBool(PROP_OVERRIDE);
	bool value = false;
	if (_override)
	{
		value = getPropertyValueBool(PROP_OVERRIDE_VALUE);
		setPropertyValueBool(PROP_VALUE,value);
	
	}
	else
	{
		float thl = getPropertyValueFloat(PROP_TH_L);
		float thh = getPropertyValueFloat(PROP_TH_H); 
		int mode = getPropertyValueInt(PROP_TH_MODE);
		if (mode == MODE_UNDER)
		{		// active when under
				if (measure <= thl)
				{
					value = true;
					setPropertyValueBool(PROP_VALUE,value);
				}
				else if (measure >= thh)
				{
					value = false;
					setPropertyValueBool(PROP_VALUE,value);
				}
		}
		else if (mode == MODE_OVER)
		{ //active when over
			if (measure <= thl)
			{
				value = false;
				setPropertyValueBool(PROP_VALUE,value);
			}
			else if (measure >= thh)
			{
				value = true;
				setPropertyValueBool(PROP_VALUE,value);
			}
		}
		else if (mode == MODE_IN)
		{ //MODE IN
			value = (measure>=thl && measure <=thh);
			setPropertyValueBool(PROP_VALUE,value);
		}
		else if (mode == MODE_OUT)
		{ //MODE OUT
			value = (measure<= thl || measure >=thh);
			setPropertyValueBool(PROP_VALUE,value);
		}
	}

	setPropertyValueFloat(PROP_TH_LAST_VALUE,measure);
	return value;
}

bool Threshold::getValue()
{
	return getPropertyValueBool(PROP_VALUE);
}


void Threshold::getThValue(float &h, float &l)
{
	l = getPropertyValueFloat(PROP_TH_L);
	h = getPropertyValueFloat(PROP_TH_H); 

}