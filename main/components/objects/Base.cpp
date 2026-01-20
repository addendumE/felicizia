
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
	setPropertyValue(p,_default);
	ESP_LOGW(TAG,"%s.%s added with value %f",id.c_str(),propertyNames.at(p).c_str(),_default);
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
	setPropertyValue(p,_default);
	ESP_LOGW(TAG,"%s.%s added with value %s",id.c_str(),propertyNames.at(p).c_str(),_default.c_str());
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
			ESP_LOGW(TAG,"%s.%s no persit value found",id.c_str(),propertyNames.at(p).c_str());
		}
	}
	setPropertyValue(p,_default);
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
	setPropertyValue(p,_default);
	ESP_LOGW(TAG,"%s.%s added with value %d",id.c_str(),propertyNames.at(p).c_str(),_default);
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
	setPropertyValue(p,_default);
	ESP_LOGW(TAG,"%s.%s added with value %d",id.c_str(),propertyNames.at(p).c_str(),_default);
	return true;
}


bool Base::getPropertyJsonValue(PropertyId p , cJSON **jObj)
{
	bool ret = true;
	switch (propertyList[p]->getType())
	{
		case VALUE_TYPE_BOOL:
		{
			bool val = getPropertyValue<bool>(p, ret);
			*jObj = cJSON_CreateBool(val);
			break;
		}
		case VALUE_TYPE_FLOAT:
		{
			float val = getPropertyValue<float>(p,ret);
			*jObj = cJSON_CreateNumber(val);
			break;
		}
		case VALUE_TYPE_INTEGER:
		case VALUE_TYPE_ENUM:
		{
			int val = getPropertyValue<int>(p, ret);
			*jObj = cJSON_CreateNumber(val);
			break;
		}
		case VALUE_TYPE_STRING:
		{
			string val = getPropertyValue<string>(p, ret);
			*jObj = cJSON_CreateString(val.c_str());
			break;
		}
		default:
			ret = false;
	}

	return ret;
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

template string Base::getPropertyValue<string>(Types::PropertyId, bool&);
template bool Base::getPropertyValue<bool>(Types::PropertyId, bool&);
template int Base::getPropertyValue<int>(Types::PropertyId, bool&);
template float Base::getPropertyValue<float>(Types::PropertyId, bool&);

template <typename T>
T Base::getPropertyValue(PropertyId p, bool &found)
{
	T ret;
	//ESP_LOGI(TAG,"get property %s %zu ",id.c_str(),p);
	if (propertyList.count(p) > 0)
	{
		T *val = (T *)propertyList[p]->get();
		ret = *val;
		delete(val);
		found = true;
	}
	else
	{
		found = false;
		ESP_LOGE(TAG,"property %zu not found in obj %s",p,id.c_str());
	}
	return ret;

}


string Base::getPropertyValueString(PropertyId p)
{
	string ret;
	if (propertyList.count(p) > 0)
	{
		ret = propertyList[p]->toString();
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
						ESP_LOGI(TAG,"Write true");
						retVal = setPropertyValue<bool>(pId,true);
					}
					else
					{
						ESP_LOGI(TAG,"Write false");
						retVal = setPropertyValue<bool>(pId,false);
					}
					break;
				case VALUE_TYPE_FLOAT:
					retVal = setPropertyValue<float>(pId,(float)atof(value.c_str()));
					break;
				case VALUE_TYPE_INTEGER:
				case VALUE_TYPE_ENUM:
					retVal = setPropertyValue<int>(pId,atoi(value.c_str()));
					break;
				case VALUE_TYPE_STRING:
					retVal = setPropertyValue<string>(pId,value);
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
template <typename T>
Property::SetResult Base::setPropertyValue(PropertyId p, T val)
{
	Property::SetResult ret = Property::SET_OK;
	if (propertyList.count(p) > 0)
	{
		string old = propertyList[p]->toString();
		ret = propertyList[p]->set((void*)&val);
		bool changed = (old!=propertyList[p]->toString());
		if (ret==Property::SET_OK && changed)
		{
			persistance.changeNotify(id, p);
			if (propertyList[p]->getPersistance())
			{
				bool stored;
				switch (propertyList[p]->getType())
				{
					case VALUE_TYPE_BOOL: stored = persistance.storeBool(id, p, *(bool*) &val); break;
					case VALUE_TYPE_INTEGER: stored = persistance.storeInt(id, p, *(int*) &val); break;
					case VALUE_TYPE_STRING: stored = persistance.storeString(id, p, *(string*) &val); break;
					case VALUE_TYPE_FLOAT: stored = persistance.storeFloat(id, p, *(float*) &val); break;
					case VALUE_TYPE_ENUM: stored = persistance.storeInt(id, p, *(int*) &val); break;
				}
				if (!stored) {
					ret = Property::SET_STORE_ERROR;
				}
			}
		}
	}
	return ret;
}

std::string Base::floatToString(float value, int decimals)
{
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(decimals) << value;
    return oss.str();
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
				ESP_LOGI(TAG,"property %s found",id.c_str());
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

