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


void ObjManager::getConf(string &s)
{
	cJSON *jOut = cJSON_CreateObject();
	for (auto iobj:objects)
	{
		map <PropertyId,Property *> properties = iobj.second->getProperties();
		cJSON *jProperties = cJSON_CreateObject();
		cJSON_AddItemToObject(jOut, iobj.second->getId().c_str(), jProperties);
		for (auto iprop:properties)
		{
			
			if (iprop.second->getPersistance())
			{
				cJSON *jProp= cJSON_CreateString(iobj.second->getPropertyValueString(iprop.first,true).c_str());
				cJSON_AddItemToObject(jProperties, getPropertyName(iprop.first), jProp);
			}
		}
	}
	char *txt = cJSON_PrintUnformatted(jOut);
	s=string(txt);
	free(txt);
	cJSON_Delete(jOut);
}
bool ObjManager::setConf(string &s)
{
	bool ret = true;
	cJSON *jObj = cJSON_Parse(s.c_str());
	if (jObj == NULL)
	{
		const char *error_ptr = cJSON_GetErrorPtr();
		if (error_ptr != NULL)
		{
			fprintf(stderr, "Error before: %s\n", error_ptr);
		}
		ret = false;
	}
	else
	{
		cJSON *jItem = jObj->child;
		while (jItem) {
		   	cJSON *jProperty = jItem->child;
			string sObj=string(jItem->string);
			Base *o = objects.at(sObj);
			if (o) {
				while(jProperty)
				{
					string sProp=string(jProperty->string);
					string sVal=string(jProperty->valuestring);
					Property::SetResult setRes = o->setProperyValueFromString(sProp,sVal);
					jProperty = jProperty->next;
				}
			}
    		jItem = jItem->next;
		}
	}
    cJSON_Delete(jObj);
	return ret;
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
		cJSON *jItem = cJSON_CreateObject();
		cJSON_AddStringToObject(jItem, "id", item.second->getId().c_str());
		cJSON_AddStringToObject(jItem, "type", item.second->getPropertyValueString(PROP_TYPE).c_str());
		cJSON_AddStringToObject(jItem, "name", item.second->getPropertyValueString(PROP_NAME).c_str());
		cJSON_AddStringToObject(jItem, "svalue", item.second->getPropertyValueString(PROP_VALUE).c_str());
		cJSON_AddStringToObject(jItem, "override", item.second->getPropertyValueString(PROP_OVERRIDE).c_str());
		cJSON_AddStringToObject(jItem, "fail", item.second->getPropertyValueString(PROP_FAIL).c_str());
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
	for (int i = 0; i < typeEntriesCount; ++i)
	{
		cJSON *jItem = cJSON_CreateString(typeEntries[i].name);
		cJSON_AddItemToArray(jOut,jItem);
	}
	ESP_LOGI(TAG,"getObjectTypes returned %d items",typeEntriesCount);

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
