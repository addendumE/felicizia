#pragma once
#include <string>
#include "Base.h"
using namespace std;
using namespace Types;

class Persistance {
public:
	Persistance();
	virtual ~Persistance();
	virtual void changeNotify(string objId, PropertyId p) = 0;
	virtual bool loadFloat(string id, PropertyId p, float &value) = 0;
	virtual bool loadInt(string id, PropertyId p, int &value) = 0;
	virtual bool loadBool(string id, PropertyId p, bool &value) = 0;
	virtual bool loadString(string id, PropertyId p, string &value) = 0;
	virtual bool storeFloat(string id, PropertyId p, float value) = 0;
	virtual bool storeInt(string id, PropertyId p, int value) = 0;
	virtual bool storeBool(string id, PropertyId p, bool value) = 0;
	virtual bool storeString(string id, PropertyId p, string value) = 0;

};

