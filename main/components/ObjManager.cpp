/*
 * ObjManager.cpp
 *
 *  Created on: Nov 25, 2025
 *      Author: maurizio
 */

#include "ObjManager.h"
#include "esp_log.h"

static const char *TAG="ObjManager";

ObjManager::ObjManager() {
}

ObjManager::~ObjManager() {
}


bool ObjManager::addObject(Base *obj)
{
	objects[obj->getId()] = obj;
	return true;
}

cJSON * ObjManager::getPropertyJSONvalue(string objId, PropertyId  propId)
{
	cJSON * ret = nullptr;
	Base * o =getObjectPtr(objId);
	if (o)
	{
		ret = o->getPropertyJson(propId);
	}
	return ret;
}

cJSON *ObjManager::getObjectList()
{
	cJSON *jOut = cJSON_CreateArray();
	for (auto item:objects)
	{
		bool found;
		cJSON *jItem = cJSON_CreateObject();
		ESP_LOGI(TAG,"id");
		cJSON_AddStringToObject(jItem, "id", item.second->getId().c_str());
		ESP_LOGI(TAG,"type");
		cJSON_AddStringToObject(jItem, "type", item.second->getPropertyValue<string>(PROP_TYPE,found).c_str());
		ESP_LOGI(TAG,"name");
		cJSON_AddStringToObject(jItem, "name", item.second->getPropertyValue<string>(PROP_NAME,found).c_str());
		ESP_LOGI(TAG,"svalue");
		cJSON_AddStringToObject(jItem, "svalue", item.second->getPropertyValueString(PROP_VALUE).c_str());
		ESP_LOGI(TAG,"override");
		cJSON_AddStringToObject(jItem, "override", item.second->getPropertyValueString(PROP_OVERRIDE).c_str());
		ESP_LOGI(TAG,"fail");
		cJSON_AddStringToObject(jItem, "fail", item.second->getPropertyValueString(PROP_FAIL).c_str());
		ESP_LOGI(TAG,"adding");
		cJSON_AddItemToArray(jOut,jItem);
	}
	return jOut;
}

cJSON *ObjManager::getObject(string id)
{

	cJSON *jItem = cJSON_CreateObject();
	cJSON_AddStringToObject(jItem,"id",id.c_str());
	if (objects.count(id))
	{
		cJSON * jOut = objects.at(id)->get();
		cJSON_AddItemToObject(jItem,"properties",jOut);
	}
	else
	{
		ESP_LOGE(TAG,"getObject object not found %s",id.c_str());
	}
	return jItem;
}

cJSON *ObjManager::getObjectTypes()
{
	cJSON *jOut = cJSON_CreateArray();
	for (auto item:Types::typeNames)
	{
		cJSON *jItem = cJSON_CreateString(item.second.c_str());
		cJSON_AddItemToArray(jOut,jItem);
	}
	ESP_LOGI(TAG,"getObjectTypes returned %zu items",typeNames.size());

	return jOut;
}

Base *ObjManager::getObjectPtr(string id)
{
	Base * p = nullptr;
	if (objects.count(id))
	{
			p = objects.at(id);
	}
	return p;
}

Property *ObjManager::getPropertyPtr(Base * obj,string propId)
{
	Property * p = nullptr;
	p = obj->getProperty(propId);
	return p;
}

void ObjManager::propChangeNotification(string objId,Types::PropertyId id)
{
	ESP_LOGI(TAG,"%s %s changed",objId.c_str(),propertyNames.at(id).c_str());
	/*Base *oPnt = getObjectPtr(objId);
	ObjManager::Bind src(oPnt,id);
	if (bingings.count(src) > 0)
	{
		for (auto p:bingings.at(src))
		{
			bool found;
			float x = oPnt->getPropertyValue<float>(id, found);
			get<0>(p)->setPropertyValue(get<1>(p),x,true);
			printf("found!!! %f",x);
		}
	}*/
}

void ObjManager::addBinding(Base * srcObj,PropertyId srcProperty, Base * dstObj,PropertyId dstProperty)
{
	ObjManager::Bind src(srcObj,srcProperty);
	ObjManager::Bind dst(dstObj,dstProperty);
	bingings[src].push_back(dst);
}
