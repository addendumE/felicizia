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
	bool _found;
	bool _override = getPropertyValue<bool>(PROP_OVERRIDE,_found);
	bool value;
	if (_override)
	{
		value = getPropertyValue<bool>(PROP_OVERRIDE_VALUE,_found);
	
	}
	else
	{
		float thl = getPropertyValue<float>(PROP_TH_L,_found);
		float thh = getPropertyValue<float>(PROP_TH_H,_found); 
		int mode = getPropertyValue<int>(PROP_TH_MODE,_found);
		if (mode == MODE_UNDER)
		{		// active when under
				if (measure < thl)
				{
					value = true;
				}
				else if (measure > thh)
				{
					value = false;
				}
		}
		else if (mode == MODE_OVER)
		{ //active when over
			if (measure > thl)
			{
				value = true;
			}
			else if (measure < thh)
			{
				value = false;
			}
		}
		else if (mode == MODE_IN)
		{ //MODE IN
			value = (measure>=thl && measure <=thh);
		}
		else if (mode == MODE_OUT)
		{ //MODE OUT
			value = (measure<= thl || measure >=thh);
		}

	}
	setPropertyValue<bool>(PROP_VALUE,value);
	setPropertyValue<float>(PROP_TH_LAST_VALUE,measure);
	return value;
}

bool Threshold::getValue()
{
	bool _found;
	return getPropertyValue<bool>(PROP_VALUE,_found);
}


void Threshold::getThValue(float &h, float &l)
{
	bool _found;
	l = getPropertyValue<float>(PROP_TH_L,_found);
	h = getPropertyValue<float>(PROP_TH_H,_found); 

}