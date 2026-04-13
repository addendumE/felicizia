
#include "Base.h"
#include "esp_log.h"
#include <string>
#include <sstream>
#include <iomanip>

static const char *TAG="Base";

Base::Base(Type _type, string id,string _name, Persistance &persistance):
		id(id),
		persistance(persistance)
{
	addStringProperty(PROP_TYPE,Property::MODE_READONLY,typeNames.at(_type));
	addStringProperty(PROP_NAME,Property::MODE_READONLY,_name);
}

Base::~Base() {
}

bool Base::addFloatProperty(PropertyId p, Property::Mode mode, float _default, Unit unit, int decimals, FloatProperty::onSet onSetCback)
{
	FloatProperty *fp = new FloatProperty(mode, unit,  decimals,onSetCback);
	propertyList[p] = fp;
	if (fp->getPersistance())
	{
		if (!persistance.loadFloat(id, p, _default))
		{
			ESP_LOGW(TAG,"%s.%s no persit value found",id.c_str(),propertyNames.at(p).c_str());
		}
	}
	setPropertyValueFloat(p,_default);
	//ESP_LOGW(TAG,"%s.%s added with value %f",id.c_str(),propertyNames.at(p).c_str(),_default);
	return true;
}
bool Base::addStringProperty(PropertyId p, Property::Mode mode, string _default, StringProperty::onSet onSetCback)
{
	StringProperty *sp = new StringProperty(mode, onSetCback);
	propertyList[p] = sp;

	if (sp->getPersistance())
	{
		if (!persistance.loadString(id, p, _default))
		{
			ESP_LOGW(TAG,"%s.%s no persit value found",id.c_str(),propertyNames.at(p).c_str());
		}
	}
	setPropertyValueString(p,_default);
	//ESP_LOGW(TAG,"%s.%s added with value %s",id.c_str(),propertyNames.at(p).c_str(),_default.c_str());
	return true;
}

bool Base::addIntProperty(PropertyId p,Property::Mode mode,  int _default, IntProperty::onSet onSetCback)
{
	IntProperty *ip = new IntProperty(mode, onSetCback);
	propertyList[p] = ip;
	if (ip->getPersistance())
	{
		if (!persistance.loadInt(id, p, _default))
		{
			ESP_LOGW(TAG,"%s.%s ",id.c_str(),propertyNames.at(p).c_str());
		}
	}
	setPropertyValueInt(p,_default);
	ESP_LOGW(TAG,"%s.%s added with value %d",id.c_str(),propertyNames.at(p).c_str(),_default);
	return true;
}

bool Base::addEnumProperty(PropertyId p,Property::Mode mode,  int _default,  vector <string> *_strings, IntProperty::onSet onSetCback)
{
	EnumProperty *ip = new EnumProperty(mode, onSetCback,_strings);
	propertyList[p] = ip;
	if (ip->getPersistance())
	{
		if (!persistance.loadInt(id, p, _default))
		{
			ESP_LOGW(TAG,"%s.%s no persit value found",id.c_str(),propertyNames.at(p).c_str());
		}
	}
	setPropertyValueInt(p,_default);
	//ESP_LOGW(TAG,"%s.%s added with value %d",id.c_str(),propertyNames.at(p).c_str(),_default);
	return true;
}

bool Base::addBoolProperty(PropertyId p,  Property::Mode mode, bool _default , string trues, string falses, BoolProperty::onSet onSetCback)
{
	BoolProperty *bp = new BoolProperty(mode, onSetCback,trues,falses);
	propertyList[p] = bp;
	if (bp->getPersistance())
	{
		if (!persistance.loadBool(id, p, _default))
		{
			ESP_LOGW(TAG,"%s.%s no persit value found",id.c_str(),propertyNames.at(p).c_str());
		}
	}
	setPropertyValueBool(p,_default);
	//ESP_LOGW(TAG,"%s.%s added with value %d",id.c_str(),propertyNames.at(p).c_str(),_default);
	return true;
}


bool Base::getPropertyJsonValue(PropertyId p , cJSON **jObj)
{
	switch (propertyList[p]->getType())
	{
		case VALUE_TYPE_BOOL:
		{
			bool val = getPropertyValueBool(p);
			*jObj = cJSON_CreateBool(val);
			break;
		}
		case VALUE_TYPE_FLOAT:
		{
			float val = getPropertyValueFloat(p);
			*jObj = cJSON_CreateNumber(val);
			break;
		}
		case VALUE_TYPE_INTEGER:
		case VALUE_TYPE_ENUM:
		{
			int val = getPropertyValueInt(p);
			*jObj = cJSON_CreateNumber(val);
			break;
		}
		case VALUE_TYPE_STRING:
		{
			string val = getPropertyValueString(p);
			*jObj = cJSON_CreateString(val.c_str());
			break;
		}
		default:
			break;
	}

	return true;
}
#if 0
Property::SetResult Base::setProperty(PropertyId p, cJSON *jObj)
{
	Property::SetResult ret = Property::SET_INTERNAL_ERROR;
	switch (propertyList[p]->getType())
	{
		case VALUE_TYPE_BOOL:
		{
			if (cJSON_IsBool(jObj))
			{
				bool val = cJSON_IsTrue(jObj);
				ret = setPropertyValue(p,val);

			}
			else ret = Property::SET_FORMAT_ERROR;
			break;
		}
		case VALUE_TYPE_FLOAT:
		{
			if (cJSON_IsNumber(jObj))
			{
				float val;
				val = (float)cJSON_GetNumberValue(jObj);
				ret = setPropertyValue(p,val);
			}
			else ret = Property::SET_FORMAT_ERROR;
			break;
		}
		case VALUE_TYPE_INTEGER:
		case VALUE_TYPE_ENUM:
		{
			if (cJSON_IsNumber(jObj))
			{
				int  val;
				val = (int)cJSON_GetNumberValue(jObj);
				ret = setPropertyValue(p,val);
			}
			else ret = Property::SET_FORMAT_ERROR;
			break;
		}
		case VALUE_TYPE_STRING:
		{
			if (cJSON_IsString(jObj))
			{
				string val;
				val = cJSON_GetStringValue(jObj);
				ret = setPropertyValue(p,val);
			}
			else ret = Property::SET_FORMAT_ERROR;
			break;
		}
		default:
			break;
	}

	return ret;
}

#endif
cJSON * Base::get()
{
	cJSON *jOut = cJSON_CreateObject();
	for (auto p:propertyList)
	{
		cJSON *jItem = getPropertyJson(p.first);
		cJSON_AddItemToObject(jOut, propertyNames.at(p.first).c_str(), jItem);
	}
    return jOut;
}

cJSON * Base::getPropertyJson(PropertyId pId)
{
	cJSON *jItem = cJSON_CreateObject();
	cJSON *jValue;
	getPropertyJsonValue(pId,&jValue);
	cJSON_AddItemToObject(jItem, "value", jValue);
	cJSON_AddBoolToObject(jItem, "rw", propertyList.at(pId)->getWritable());
	cJSON_AddBoolToObject(jItem, "persist", propertyList.at(pId)->getPersistance());
	cJSON_AddNumberToObject(jItem, "type", propertyList.at(pId)->getType());
	cJSON_AddStringToObject(jItem, "svalue", (propertyList.at(pId)->toString()).c_str());
	if (propertyList.at(pId)->getType() == VALUE_TYPE_ENUM)
	{
		cJSON *jArray = cJSON_CreateArray();
		EnumProperty *p = (EnumProperty *)(propertyList.at(pId));
		vector <string> &strings = p->getStrings();
		for (auto o:strings)
		{
		    cJSON *sJson = cJSON_CreateString(o.c_str());
			cJSON_AddItemToArray(jArray,sJson);
		}
		cJSON_AddItemToObject(jItem, "options",jArray);
	}
	return jItem;
}


string Base::serialize()
{
	cJSON * jOut = get();
	char *txt = cJSON_Print(jOut);
	string res;
	res = txt;
	free(txt);
	cJSON_Delete(jOut);
	return res;
}

#if 0
bool Base::setProperty(string data)
{
	 cJSON *jObj = cJSON_Parse(data.c_str());
	 if (jObj == NULL)
	 {
	   const char *error_ptr = cJSON_GetErrorPtr();
	   if (error_ptr != NULL)
	   {
		   fprintf(stderr, "Error before: %s\n", error_ptr);
	   }
	   return false;
	 }
	 bool ret = false;
	 jObj = jObj->child;
	 //look up for propery

	 for (auto p:propertyList)
	 {
		 if (propertyNames.at(p.first)==string(jObj->string))
		{
			ret = setProperty(p.first,jObj);

		}
	 }
	 cJSON_Delete(jObj);
	return ret;
}
#endif


bool Base::getPropertyValueBool(PropertyId p)
{
	bool ret = false;
	if (propertyList.count(p) > 0)
	{
		ret = static_cast<BoolProperty*>(propertyList[p])->get();
	}
	else
	{
		ESP_LOGE(TAG,"property %zu not found in obj %s",p,id.c_str());
	}
	return ret;

}

float Base::getPropertyValueFloat(PropertyId p)
{
	float ret = 0.0f;
	if (propertyList.count(p) > 0)
	{
		ret = static_cast<FloatProperty*>(propertyList[p])->get();
	}
	else
	{
		ESP_LOGE(TAG,"property %zu not found in obj %s",p,id.c_str());
	}
	return ret;

}


int Base::getPropertyValueInt(PropertyId p)
{
	int ret = 0;
	if (propertyList.count(p) > 0)
	{
		ret = static_cast<IntProperty*>(propertyList[p])->get();
	}
	else
	{
		ESP_LOGE(TAG,"property %zu not found in obj %s",p,id.c_str());
	}
	return ret;

}

string Base::getPropertyValueString(PropertyId p, bool raw)
{
	string ret;
	if (propertyList.count(p) > 0)
	{
		ret = propertyList[p]->toString(raw);
	}
	else
	{
		ESP_LOGE(TAG,"property %zu not found in obj %s",p,id.c_str());
	}
	return ret;
}
Property::SetResult Base::setProperyValueFromString(string propId, string value)
{
	Property::SetResult retVal = Property::SET_INTERNAL_ERROR;
	bool ret = false;
	PropertyId pId;
	for (auto p:propertyNames)
	{
		if (p.second == propId)
		{
			ret = true;
			pId = p.first;
			break;
		}
	}
	if (ret)
	{
		if (propertyList.count(pId))
		{
			Property *pPnt = propertyList.at(pId);
			switch (pPnt->getType())
			{
				case VALUE_TYPE_BOOL:
					if (value=="true")
					{
						retVal = setPropertyValueBool(pId,true);
					}
					else
					{
						retVal = setPropertyValueBool(pId,false);
					}
					break;
				case VALUE_TYPE_FLOAT:
					retVal = setPropertyValueFloat(pId,(float)atof(value.c_str()));
					break;
				case VALUE_TYPE_INTEGER:
				case VALUE_TYPE_ENUM:
					retVal = setPropertyValueInt(pId,atoi(value.c_str()));
					break;
				case VALUE_TYPE_STRING:
					retVal = setPropertyValueString(pId,value);
					break;
			}

		}
		else
		{
			ESP_LOGE(TAG,"%s has not %s property",id.c_str(),propId.c_str());
			retVal = Property::SET_NOTFOUND_ERROR;
		}

	}
	else
	{
		ESP_LOGE(TAG,"%s not property",propId.c_str());
		retVal = Property::SET_NOT_PROPERTY_ERROR;
	}
	return retVal;
}

Property::SetResult Base::setPropertyValueBool(PropertyId pid, bool val)
{
	Property::SetResult ret = Property::SET_OK;
	if (propertyList.count(pid) > 0)
	{
		if (propertyList[pid]->getType()!=VALUE_TYPE_BOOL) return Property::SET_WRONG_TYPE;
		BoolProperty * p = static_cast<BoolProperty*>(propertyList[pid]);
		bool old = p->get();
		ret = p->set(val);
		bool changed = (old!=val);
		if (ret==Property::SET_OK && changed)
		{
			persistance.changeNotify(id, pid);
			if (p->getPersistance())
			{
				bool stored =  persistance.storeBool(id, pid, val);
				if (!stored) {
					ret = Property::SET_STORE_ERROR;
				}
			}
		}
	}
	return ret;
}

Property::SetResult Base::setPropertyValueInt(PropertyId pid, int val)
{
	Property::SetResult ret = Property::SET_OK;
	if (propertyList.count(pid) > 0)
	{
		if (propertyList[pid]->getType()!=VALUE_TYPE_INTEGER) return Property::SET_WRONG_TYPE;
		IntProperty * p = static_cast<IntProperty*>(propertyList[pid]);
		int old = p->get();
		ret = p->set(val);
		bool changed = (old!=val);
		if (ret==Property::SET_OK && changed)
		{
			persistance.changeNotify(id, pid);
			if (p->getPersistance())
			{
				bool stored =  persistance.storeInt(id, pid, val);
				if (!stored) {
					ret = Property::SET_STORE_ERROR;
				}
			}
		}
	}
	return ret;
}

Property::SetResult Base::setPropertyValueString(PropertyId pid, const string &val)
{
	Property::SetResult ret = Property::SET_OK;
	if (propertyList.count(pid) > 0)
	{
		if (propertyList[pid]->getType()!=VALUE_TYPE_STRING) return Property::SET_WRONG_TYPE;
		StringProperty * p = static_cast<StringProperty*>(propertyList[pid]);
		string old = p->get();
		ret = p->set(val);
		bool changed = (old!=val);
		if (ret==Property::SET_OK && changed)
		{
			persistance.changeNotify(id, pid);
			if (p->getPersistance())
			{
				bool stored =  persistance.storeString(id, pid, val);
				if (!stored) {
					ret = Property::SET_STORE_ERROR;
				}
			}
		}
	}
	return ret;

}


Property::SetResult Base::setPropertyValueFloat(PropertyId pid, float val)
{
	Property::SetResult ret = Property::SET_OK;
	if (propertyList.count(pid) > 0)
	{
		if (propertyList[pid]->getType()!=VALUE_TYPE_FLOAT) return Property::SET_WRONG_TYPE;
		FloatProperty * p = static_cast<FloatProperty*>(propertyList[pid]);
		float old = p->get();
		ret = p->set(val);
		bool changed = (old!=val);
		if (ret==Property::SET_OK && changed)
		{
			persistance.changeNotify(id, pid);
			if (p->getPersistance())
			{
				bool stored =  persistance.storeFloat(id, pid, val);
				if (!stored) {
					ret = Property::SET_STORE_ERROR;
				}
			}
		}
	}
	return ret;
}

Property *Base::getProperty(string id)
{
	Property * ret = nullptr;
	for (auto p:propertyNames)
	{
		if (p.second == id)
		{
			if (propertyList.count(p.first)>0)
			{
				ret = propertyList.at(p.first);
				//ESP_LOGI(TAG,"property %s found",id.c_str());
			}
			else
			{
				ESP_LOGE(TAG,"property %s not found",id.c_str());
			}
			break;
		}
	}
	return ret;
}

