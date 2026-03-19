/*
 * Device.cpp
 *
 *  Created on: Nov 24, 2025
 *      Author: maurizio
 */

#include "Device.h"

Device::Device(string id,string name, Persistance &p):
	Base(TYPE_DEVICE,id,name,p)
{
	addStringProperty(PROP_VALUE,Property::MODE_READONLY,"OK");
	addFloatProperty(PROP_FREE_HEAP,Property::MODE_READONLY, 0,UNIT_PERCENT,1);
	addStringProperty(PROP_SSID,Property::MODE_WRITABLE,"");
	addStringProperty(PROP_SSID_KEY,Property::MODE_WRITABLE,"");
	addStringProperty(PROP_TS_KEY,Property::MODE_WRITABLE,"");
}

Device::~Device() {
}

void Device::setFreeHeap(float fh)
{
	setPropertyValue<float>(PROP_FREE_HEAP,fh);
}