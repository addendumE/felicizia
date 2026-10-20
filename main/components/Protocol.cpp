/*
 * Protocol.cpp
 *
 *  Created on: Nov 25, 2025
 *      Author: maurizio
 */

#include "Protocol.h"
#include "esp_log.h"
#include <cstdint>
#include <vector>

#ifndef LINUX_PLATFORM
#include "esp_core_dump.h"
#include "esp_partition.h"
#endif

static const char *TAG="PROTOCOL";

#ifndef LINUX_PLATFORM
static const char b64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static std::string base64_encode(const uint8_t *data, size_t len) {
	std::string out;
	out.reserve(((len + 2) / 3) * 4);
	for (size_t i = 0; i < len; i += 3) {
		uint32_t b = ((uint32_t)data[i]) << 16;
		if (i + 1 < len) b |= ((uint32_t)data[i + 1]) << 8;
		if (i + 2 < len) b |= ((uint32_t)data[i + 2]);

		out.push_back(b64_chars[(b >> 18) & 0x3F]);
		out.push_back(b64_chars[(b >> 12) & 0x3F]);
		out.push_back((i + 1 < len) ? b64_chars[(b >> 6) & 0x3F] : '=');
		out.push_back((i + 2 < len) ? b64_chars[b & 0x3F] : '=');
	}
	return out;
}
#endif

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
	if (txt) {
		msg = txt;
		free(txt);
	}
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
	else if (cmd == "getCoredumpInfo")
	{
		res = handleCoredumpInfo(jDataReq, &jDataResp);
	}
	else if (cmd == "getCoredumpPart")
	{
		res = handleCoredumpPart(jDataReq, &jDataResp);
	}
	else if (cmd == "eraseCoredump")
	{
		res = handleCoredumpErase(jDataReq, &jDataResp);
	}
	//else if (cmd == "reboot")
	//{
		//handleConfWrite(jDataReq,&jDataResp);
		//res = RES_OK;
	//}
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
	string result;
	if (txt) {
		result = txt;
		free(txt);
	}
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

Protocol::handleResult Protocol::handleCoredumpInfo(cJSON *jReq, cJSON **jResp)
{
	*jResp = cJSON_CreateObject();
#ifndef LINUX_PLATFORM
	esp_err_t err = esp_core_dump_image_check();
	if (err != ESP_OK) {
		cJSON_AddBoolToObject(*jResp, "hasDump", false);
		return RES_OK;
	}
	size_t addr = 0, size = 0;
	if (esp_core_dump_image_get(&addr, &size) != ESP_OK || size == 0) {
		cJSON_AddBoolToObject(*jResp, "hasDump", false);
		return RES_OK;
	}
	cJSON_AddBoolToObject(*jResp, "hasDump", true);
	cJSON_AddNumberToObject(*jResp, "size", size);
#if CONFIG_ESP_COREDUMP_ENABLE_TO_FLASH && CONFIG_ESP_COREDUMP_DATA_FORMAT_ELF
	char reason[128] = {0};
	if (esp_core_dump_get_panic_reason(reason, sizeof(reason)) == ESP_OK) {
		cJSON_AddStringToObject(*jResp, "reason", reason);
	}
	esp_core_dump_summary_t summary;
	if (esp_core_dump_get_summary(&summary) == ESP_OK) {
		cJSON_AddStringToObject(*jResp, "task", summary.exc_task);
		cJSON_AddNumberToObject(*jResp, "pc", summary.exc_pc);
	}
#endif
#else
	cJSON_AddBoolToObject(*jResp, "hasDump", false);
#endif
	return RES_OK;
}

Protocol::handleResult Protocol::handleCoredumpPart(cJSON *jReq, cJSON **jResp)
{
#ifndef LINUX_PLATFORM
	esp_err_t err = esp_core_dump_image_check();
	if (err != ESP_OK) {
		return RES_ERR_INTERNAL;
	}
	size_t addr = 0, total_size = 0;
	if (esp_core_dump_image_get(&addr, &total_size) != ESP_OK || total_size == 0) {
		return RES_ERR_INTERNAL;
	}

	size_t offset = 0;
	if (jReq) {
		cJSON *jOff = cJSON_GetObjectItem(jReq, "offset");
		if (cJSON_IsNumber(jOff) && jOff->valueint >= 0) {
			offset = (size_t)jOff->valueint;
		}
	}

	size_t chunk_len = 1024;
	if (jReq) {
		cJSON *jLen = cJSON_GetObjectItem(jReq, "length");
		if (cJSON_IsNumber(jLen) && jLen->valueint > 0 && jLen->valueint <= 2048) {
			chunk_len = (size_t)jLen->valueint;
		}
	}

	if (offset >= total_size) {
		return RES_ERR_INTERNAL;
	}
	if (offset + chunk_len > total_size) {
		chunk_len = total_size - offset;
	}

	const esp_partition_t *part = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_COREDUMP, NULL);
	if (!part) {
		return RES_ERR_INTERNAL;
	}

	std::vector<uint8_t> buffer(chunk_len);
	if (esp_partition_read(part, offset, buffer.data(), chunk_len) != ESP_OK) {
		return RES_ERR_INTERNAL;
	}

	*jResp = cJSON_CreateObject();
	cJSON_AddNumberToObject(*jResp, "offset", offset);
	cJSON_AddNumberToObject(*jResp, "chunkSize", chunk_len);
	cJSON_AddNumberToObject(*jResp, "totalSize", total_size);
	cJSON_AddStringToObject(*jResp, "data", base64_encode(buffer.data(), chunk_len).c_str());
	return RES_OK;
#else
	return RES_ERR_INTERNAL;
#endif
}

Protocol::handleResult Protocol::handleCoredumpErase(cJSON *jReq, cJSON **jResp)
{
	*jResp = cJSON_CreateObject();
#ifndef LINUX_PLATFORM
	esp_err_t err = esp_core_dump_image_erase();
	if (err == ESP_OK) {
		cJSON_AddStringToObject(*jResp, "status", "erased");
		return RES_OK;
	} else {
		cJSON_AddStringToObject(*jResp, "error", esp_err_to_name(err));
		return RES_ERR_INTERNAL;
	}
#else
	cJSON_AddStringToObject(*jResp, "status", "erased");
	return RES_OK;
#endif
}

