#pragma once

#include <string>
#include <vector>
#include <utility>
#include "cJSON.h"
#include "Types.h"
#include "Persistance.h"
#include "Property.h"

using namespace std;
using namespace Types;

class Persistance;

class PropertyList {
public:
	using PropertyPair = std::pair<PropertyId, Property*>;
	using Container = std::vector<PropertyPair>;
	using const_iterator = Container::const_iterator;
	using iterator = Container::iterator;

	const_iterator begin() const { return items.begin(); }
	const_iterator end() const { return items.end(); }
	iterator begin() { return items.begin(); }
	iterator end() { return items.end(); }

	size_t count(PropertyId id) const {
		for (const auto &item : items) {
			if (item.first == id) return 1;
		}
		return 0;
	}

	Property* at(PropertyId id) const {
		for (const auto &item : items) {
			if (item.first == id) return item.second;
		}
		return nullptr;
	}

	Property* operator[](PropertyId id) const {
		return at(id);
	}

	Property*& operator[](PropertyId id) {
		for (auto &item : items) {
			if (item.first == id) return item.second;
		}
		items.push_back({id, nullptr});
		return items.back().second;
	}

	size_t size() const { return items.size(); }
	bool empty() const { return items.empty(); }
	void clear() { items.clear(); }

private:
	Container items;
};

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
	const PropertyList& getProperties() const { return propertyList; }
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
	PropertyList propertyList;
	Persistance &persistance;
	bool getPropertyJsonValue(PropertyId, cJSON **);
	Property::SetResult setProperty(PropertyId, cJSON *,bool &changed);


};
