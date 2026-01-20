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
	Property *getPropertyPtr(Base * obj,string propId);
	Base *getObjectPtr(string id);
	Property *getPropertyPtr(string objId,PropertyId propId)
	{
		Base * oPnt = getObjectPtr(objId);
		if (oPnt)
		{
			
			return getPropertyPtr(oPnt, propertyNames.at(propId));
		}
		else
		{
			return NULL;
		}
	};
	void propChangeNotification(string objId,Types::PropertyId id);
	void addBinding(Base * srcObj,PropertyId srcProperty, Base * dstObj,PropertyId dstProperty);
private:
	typedef tuple <Base * , PropertyId> Bind;

	map <string,Base *> objects;
	map <Bind,vector<Bind>> bingings;
};
