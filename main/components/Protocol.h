#pragma once

#include "ObjManager.h"
#include "Websocket.h"
#include "Types.h"
#include "iostream"
class Protocol:public Websocket {
public:
	Protocol(ObjManager &);
	virtual ~Protocol();
	void propChangeNotification(string objId,Types::PropertyId);
private:
	enum handleResult
	{
		RES_OK,
		RES_ERR_OBJ_NOT_FOUND,
		RES_ERR_PROP_NOT_FOUND,
		RES_ERR_SET_ERROR,
		RES_ERR_INTERNAL
	};
	ObjManager &om;
	void onMessage(const string&);
	void onOTAenter();
	void onOTAexit();
	handleResult handleObjList(cJSON *jReq,cJSON **jResp);
	handleResult handleObjTypes(cJSON *jReq,cJSON **jResp);
	handleResult handleObj(cJSON *jReq,cJSON **jResp);
	handleResult handleProperySet(cJSON *jReq,cJSON **jResp);

};
