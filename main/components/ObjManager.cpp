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
	cJSON *jOut = cJSON_CreateArray();
	for (auto iobj:objects)
	{
		map <PropertyId,Property *> properties = iobj.second->getProperties();
		for (auto iprop:properties)
		{
			if (iprop.second->getPersistance())
			{
				cJSON *jItem = cJSON_CreateObject();
				cJSON_AddStringToObject(jItem, "obj", iobj.second->getId().c_str());
				cJSON_AddStringToObject(jItem, "prop", propertyNames.at(iprop.first).c_str());
				cJSON_AddStringToObject(jItem, "value",iobj.second->getPropertyValueString(iprop.first,true).c_str());
				cJSON_AddItemToArray(jOut,jItem);
			}
		}
	}
	char *txt = cJSON_Print(jOut);
	ESP_LOGI(TAG,"CONFIGURATION: %s",txt);
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
		int size = cJSON_GetArraySize(jObj);
    	for (int i = 0; i < size; i++) {
        	cJSON *item = cJSON_GetArrayItem(jObj, i);
			cJSON *jTmp = cJSON_GetObjectItem(item,"obj");
			string objId = cJSON_GetStringValue(jTmp);
			jTmp = cJSON_GetObjectItem(item,"prop");
			string propId = cJSON_GetStringValue(jTmp);
			jTmp = cJSON_GetObjectItem(item,"value");
			string value = cJSON_GetStringValue(jTmp);
			Base *o = objects.at(objId);
			if (o) {
				Property::SetResult setRes = o->setProperyValueFromString(propId,value);
			}

		}
        	cJSON_Delete(jObj);

	}
	return true;
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
		cJSON_AddStringToObject(jItem, "id", item.second->getId().c_str());
		cJSON_AddStringToObject(jItem, "type", item.second->getPropertyValue<string>(PROP_TYPE,found).c_str());
		cJSON_AddStringToObject(jItem, "name", item.second->getPropertyValue<string>(PROP_NAME,found).c_str());
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
	//ESP_LOGI(TAG,"%s %s changed",objId.c_str(),propertyNames.at(id).c_str());
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
