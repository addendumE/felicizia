#pragma once

#include <string>
#include <map>
#include <vector>
#include "cJSON.h"
#include "Types.h"
#include "Persistance.h"
#include "Property.h"

using namespace std;
using namespace Types;

class Persistance;

class Base {
public:


	Base(Type, string, string, Persistance &persistance);
	virtual ~Base();

	bool addFloatProperty(PropertyId p, Property::Mode mode = Property::MODE_READONLY, float _default = 0.0f, Unit unit = UNIT_NONE, int decimals = 2,FloatProperty::onSet onSetCback = nullptr);
	bool addStringProperty(PropertyId p,  Property::Mode mode = Property::MODE_READONLY, string _default = "", StringProperty::onSet onSetCback=nullptr);
	bool addIntProperty(PropertyId p,  Property::Mode mode = Property::MODE_READONLY, int _default = 0, IntProperty::onSet onSetCback=nullptr);
	bool addBoolProperty(PropertyId p,  Property::Mode mode = Property::MODE_READONLY, bool _default = false, string trues="true", string falses="false",BoolProperty::onSet onSetCback=nullptr);
    bool addEnumProperty(PropertyId p,Property::Mode mode,  int _default,  vector <string> *_strings, IntProperty::onSet onSetCback = nullptr);


	string serialize();
	cJSON * get();
	map <PropertyId,Property *> getProperties() {return propertyList;};
	string getId() {return id;};
	bool getPropertyValueBool(PropertyId);
	Property::SetResult setPropertyValueBool(PropertyId pid, bool val);
	int getPropertyValueInt(PropertyId);
	Property::SetResult setPropertyValueInt(PropertyId pid, int val);
	float getPropertyValueFloat(PropertyId);
	Property::SetResult setPropertyValueFloat(PropertyId pid, float val);
	string getPropertyValueString(PropertyId, bool raw=false);
	Property::SetResult setPropertyValueString(PropertyId pid, const string &val);
	cJSON * getPropertyJson(PropertyId pId);
	Property::SetResult setProperyValueFromString(string propId, string value);
	Property *getProperty(string);

protected:
	std::string floatToString(float value, int decimals);


private:
	string id;
	struct PropertyAttr
	{
		ValueType type;
		bool persist;
		bool rw;
	};
	map <PropertyId,Property *> propertyList;
	Persistance &persistance;
	bool getPropertyJsonValue(PropertyId, cJSON **);
	Property::SetResult setProperty(PropertyId, cJSON *,bool &changed);


};
