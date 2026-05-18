#pragma once

#include <map>
#include "Base.h"

using namespace std;

class ObjManager {
public:
	ObjManager();
	virtual ~ObjManager();
	bool addObject(Base *obj);
	cJSON * getPropertyJSONvalue(string objId, PropertyId  propId);
	cJSON * getObjectList();
	cJSON * getObjectTypes();
	cJSON * getObject(string);
	void getConf(string &);
	bool setConf(string &);
	Property *getPropertyPtr(Base * obj,string propId);
	Base *getObjectPtr(string id);
	Property *getPropertyPtr(string objId,PropertyId propId)
	{
		Base * oPnt = getObjectPtr(objId);
		if (oPnt)
		{
			
			return getPropertyPtr(oPnt, getPropertyName(propId));
		}
		else
		{
			return NULL;
		}
	};
private:
	map <string,Base *> objects;
};
