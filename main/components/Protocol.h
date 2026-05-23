#pragma once

#include "ObjManager.h"
#include "Types.h"
#include "Lock.h"
class Protocol {
public:
	Protocol(ObjManager &);
	virtual ~Protocol();
	void propChangeNotification(string objId,Types::PropertyId);
	string commitPropChange();

	string onMessage(const string&);
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
	cJSON *jPropChangeArray;
	Lock lck;
	handleResult handleObjList(cJSON *jReq,cJSON **jResp);
	handleResult handleObjTypes(cJSON *jReq,cJSON **jResp);
	handleResult handleObj(cJSON *jReq,cJSON **jResp);
	handleResult handleProperySet(cJSON *jReq,cJSON **jResp);
	handleResult handleConfRead(cJSON *jReq,cJSON **jResp);
	handleResult handleConfWrite(cJSON *jReq,cJSON **jResp);

};
