/*
 * Protocol.cpp
 *
 *  Created on: Nov 25, 2025
 *      Author: maurizio
 */

#include "Protocol.h"
#include "esp_log.h"
static const char *TAG="PROTOCOL";

Protocol::Protocol(ObjManager &om):
om(om),
jPropChangeArray(nullptr)
{
}

Protocol::~Protocol() {
}


string Protocol::commitPropChange()
{
	string msg;
	lck.take();
	cJSON *jResp = cJSON_CreateObject();
	cJSON_AddItemToObject(jResp, "data", jPropChangeArray);
	cJSON_AddStringToObject(jResp, "cmd", "propChange");
	char *txt = cJSON_PrintUnformatted(jResp);
	msg = txt;
	free(txt);
	cJSON_Delete(jResp);
	jPropChangeArray = nullptr;
	lck.give();
	return msg;
}

void Protocol::propChangeNotification(const string objId,Types::PropertyId id)
{
	lck.take();
	if (jPropChangeArray == nullptr) jPropChangeArray = cJSON_CreateArray();
	cJSON * jProp = om.getPropertyJSONvalue(objId,id);
	if (jProp)
	{
		string sid = objId+"_"+getPropertyName(id);
		cJSON_AddStringToObject(jProp, "id", sid.c_str());
		cJSON_AddItemToArray(jPropChangeArray,jProp);
	}
	lck.give();
}

string Protocol::onMessage(const string &msg)
{
	cJSON *jReq = cJSON_Parse(msg.c_str());
	cJSON *jResp = cJSON_CreateObject();
	if (jReq == NULL)
	{
		const char *pEerr = cJSON_GetErrorPtr();
		if (pEerr)
		{
			ESP_LOGE(TAG,"onMessage parsing error: %s\n",pEerr);
		}
		cJSON_Delete(jResp);
		return "";
	}

	string id;
	cJSON * jTmp = cJSON_GetObjectItem(jReq,"id");
	if (cJSON_IsString(jTmp))
	{
		id = cJSON_GetStringValue(jTmp);
	}
	string cmd;
	jTmp = cJSON_GetObjectItem(jReq,"cmd");
	if (cJSON_IsString(jTmp))
	{
		cmd = cJSON_GetStringValue(jTmp);
	}
	cJSON * jDataReq = cJSON_GetObjectItem(jReq,"data");
	if (!cJSON_IsObject(jDataReq))
	{
		jDataReq = NULL;
	}

	cJSON *jDataResp = NULL;

	handleResult res = RES_ERR_INTERNAL;
	if (cmd == "getObjectList")
	{
		res = handleObjList(jDataReq,&jDataResp);
	}
	else if (cmd == "getObjectTypes")
	{
		res = handleObjTypes(jDataReq,&jDataResp);
	}
	else if (cmd == "getObject")
	{
		res = handleObj(jDataReq,&jDataResp);
	}
	else if (cmd == "setProperty")
	{
		ESP_LOGI(TAG,"onMessage %s",msg.c_str());

		res = handleProperySet(jDataReq,&jDataResp);
	}
	else if (cmd == "ping")
	{
		res = RES_OK;
	}
	else if (cmd == "readConf")
	{
		handleConfRead(jDataReq,&jDataResp);
		res = RES_OK;
	}
	else if (cmd == "writeConf")
	{
		handleConfWrite(jDataReq,&jDataResp);
		res = RES_OK;
	}
	if (jDataResp)
	{
		cJSON_AddItemToObject(jResp,"data",jDataResp);
	}

	cJSON_AddStringToObject(jResp, "id",id.c_str());
	cJSON_AddStringToObject(jResp, "cmd",cmd.c_str());
	switch (res)
	{
		case RES_OK:
			cJSON_AddStringToObject(jResp, "result","OK");
			break;
		case RES_ERR_OBJ_NOT_FOUND:
			cJSON_AddStringToObject(jResp, "result","ERR_OBJ_NOT_FOUND");
			break;
		case RES_ERR_PROP_NOT_FOUND:
			cJSON_AddStringToObject(jResp, "result","ERR_PROP_NOT_FOUND");
			break;
		case RES_ERR_INTERNAL:
			cJSON_AddStringToObject(jResp, "result","ERR_INTERNAL");
			break;
		case RES_ERR_SET_ERROR:
			cJSON_AddStringToObject(jResp, "result","RES_ERR_SET_ERROR");
			break;
	}
	cJSON_Delete(jReq);
	char *txt = cJSON_PrintUnformatted(jResp);
	cJSON_Delete(jResp);
	string result = txt;
	free(txt);
	return result;
}

Protocol::handleResult Protocol::handleObjList(cJSON *jReq,cJSON **jResp)
{
	*jResp = om.getObjectList();
	return RES_OK;
}

Protocol::handleResult Protocol::handleObjTypes(cJSON *jReq,cJSON **jResp)
{
	*jResp = om.getObjectTypes();
	return RES_OK;
}

Protocol::handleResult Protocol::handleObj(cJSON *jReq,cJSON **jResp)
{
	cJSON *jTmp = cJSON_GetObjectItem(jReq,"objId");
	if (cJSON_IsString(jTmp))
	{
		string id=cJSON_GetStringValue(jTmp);
		*jResp = om.getObject(id);
	}
	else
	{
		ESP_LOGE(TAG,"format error");
	}
	return (*jResp!=nullptr) ? RES_OK:RES_ERR_OBJ_NOT_FOUND;
}
Protocol::handleResult Protocol::handleProperySet(cJSON *jReq,cJSON **jResp)
{
	Protocol::handleResult ret = RES_OK;
	cJSON *jTmp = cJSON_GetObjectItem(jReq,"objId");
	string objId = cJSON_GetStringValue(jTmp);
	jTmp = cJSON_GetObjectItem(jReq,"propId");
	string propId = cJSON_GetStringValue(jTmp);
	jTmp = cJSON_GetObjectItem(jReq,"value");
	string value = cJSON_GetStringValue(jTmp);
	Base *o = om.getObjectPtr(objId);
	if (o)
	{
		Property::SetResult setRes = o->setProperyValueFromString(propId,value);
		if (setRes != Property::SET_OK)
		{
			ret = RES_ERR_SET_ERROR;
			string error;
			switch (setRes)
			{
				case Property::SET_FORMAT_ERROR: error="FORMAT"; break;
				case Property::SET_RANGE_ERROR: error="RANGE"; break;
				case Property::SET_INTERNAL_ERROR: error="STORE"; break;
				case Property::SET_NOTFOUND_ERROR: error="NOTFOUND"; break;
				case Property::SET_NOT_PROPERTY_ERROR: error="NOT PROPERTY"; break;
				case Property::SET_STORE_ERROR: error="STORE PROPERTY"; break;
				case Property::SET_WRONG_TYPE: error="WRONG TYPE"; break;
				case Property::SET_OK: break;
			}
			*jResp = cJSON_CreateObject();
			cJSON_AddStringToObject(*jResp,"error",error.c_str());
			ret = RES_ERR_SET_ERROR;
		}
	}
	else
	{
		ret = RES_ERR_OBJ_NOT_FOUND;
	}
	return ret;
}

Protocol::handleResult Protocol::handleConfRead(cJSON *jReq,cJSON **jResp)
{
	Protocol::handleResult ret = RES_OK;
	string conf;
	om.getConf(conf);
	*jResp = cJSON_CreateString(conf.c_str());
	return ret;
}

Protocol::handleResult Protocol::handleConfWrite(cJSON *jReq,cJSON **jResp)
{
	Protocol::handleResult ret = RES_OK;
	cJSON *jTmp = cJSON_GetObjectItem(jReq,"config");
	if (cJSON_IsString(jTmp))
	{
		string conf=cJSON_GetStringValue(jTmp);
		ret = om.setConf(conf) ? RES_OK:RES_ERR_INTERNAL;
	}
	else
	{
		ESP_LOGE(TAG,"format error");
		ret = RES_ERR_INTERNAL;
	}
	return ret;
}
	

