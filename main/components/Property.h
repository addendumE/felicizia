#pragma once
#include "Types.h"
#include <functional>
#include <sstream>
#include <iomanip>

using namespace std;
using namespace Types;

class Property {
public:

	enum Mode
	{
		MODE_READONLY,
		MODE_WRITABLE,
		MODE_WRITABLE_PERSISTENT,
	};

	enum SetResult
	{
		SET_OK,
		SET_FORMAT_ERROR,
		SET_RANGE_ERROR,
		SET_INTERNAL_ERROR,
		SET_NOTFOUND_ERROR,
		SET_NOT_PROPERTY_ERROR,
		SET_STORE_ERROR
	};

	Property(ValueType type, Mode mode):
		type(type),
		mode(mode)
	{

	};
	virtual ~Property()
	{
	};

	virtual auto get() -> void* = 0;;
	virtual SetResult set(void *) = 0;
	virtual string toString() = 0;
	ValueType getType(){return type;};
	bool getPersistance(){return mode == MODE_WRITABLE_PERSISTENT;};
	bool getWritable() {return mode != MODE_READONLY;};

private:
	ValueType type;
	Mode mode;
};

class StringProperty:public Property
{
public:
	using onSet = function <SetResult(string)>;
	StringProperty(Mode mode = MODE_READONLY, onSet onSetCback = nullptr):
		Property(VALUE_TYPE_STRING,mode),
		onSetCback(onSetCback),
		value("")
	{};
	virtual ~StringProperty(){};
	void* get() override {
		return new std::string(value);
	}

	string toString() override {
			return value;
	}

	SetResult set(void* _value) override {
		SetResult ret = SET_OK;
		std::string val = *static_cast<std::string*>(_value);
		if(onSetCback)
		{
			ret = onSetCback(val);
		}
		if (ret == SET_OK)
		{
			value = val;
		}
		return ret;
	}

private:
	onSet onSetCback;
	string value;
};


class FloatProperty:public Property
{
public:
	using onSet = function <SetResult(float)>;
	FloatProperty(Mode mode = MODE_READONLY, Unit unit = UNIT_NONE , int decimals = 2,onSet onSetCback = nullptr):
		Property(VALUE_TYPE_FLOAT, mode),
		onSetCback(onSetCback),
		value(0.0f),
		unit(unit),
		decimals(decimals)
	{};
	virtual ~FloatProperty(){};
	void* get() override {
		return new float(value);
	}

	string toString() override {
		std::ostringstream oss;
		oss << std::fixed << std::setprecision(decimals) << value;
		string ret = oss.str();
		return ret+" "+unitNames.at(unit);
	}

	SetResult set(void* _value) override {
		SetResult ret = SET_OK;
		float val = *static_cast<float*>(_value);
		if(onSetCback)
		{
			ret =  onSetCback(val);
		}
		if (ret == SET_OK)
		{
			value = val;
		}
		return ret;
	}

private:
	onSet onSetCback;
	float value;
	Unit unit;
	int decimals;
};

class IntProperty:public Property
{
public:
	using onSet = function <SetResult(int)>;
	IntProperty(Mode mode = MODE_READONLY, onSet onSetCback = nullptr):
		Property(VALUE_TYPE_INTEGER, mode),
		onSetCback(onSetCback),
		value(0)
	{};
	virtual ~IntProperty(){};
	void* get() override {
			return new int(value);
	}

	string toString() override {
		return to_string(value);
	}


	SetResult set(void* _value) override {
		SetResult ret = SET_OK;
		int val = *static_cast<int*>(_value);
		if(onSetCback)
		{
			ret = onSetCback(val);
		}
		if (ret == SET_OK)
		{
			value = val;
		}
		return ret;
	}

private:
	onSet onSetCback;
	int value;
};

class BoolProperty:public Property
{
public:
	using onSet = function <SetResult(bool)>;
	BoolProperty(Mode mode = MODE_READONLY, onSet onSetCback = nullptr, string trues="true", string falses="false"):
		Property(VALUE_TYPE_BOOL, mode),
		onSetCback(onSetCback),
		value(false),
		trues(trues),
		falses(falses)
	{};
	virtual ~BoolProperty(){};
	void* get() override {
		return new bool(value);
	}

	string toString() override {
		return (value) ? trues:falses;
	}

	SetResult set(void* _value) override {
		SetResult ret = SET_OK;
		bool val = *static_cast<bool*>(_value);
		if(onSetCback)
		{
			ret = onSetCback(val);
		}
		if (ret == SET_OK)
		{
			value = val;
		}
		return ret;
	}

private:
	onSet onSetCback;
	bool value;
	string trues;
	string falses;
};

class EnumProperty:public Property
{
public:
	using onSet = function <SetResult(int)>;
	EnumProperty(Mode mode = MODE_READONLY, onSet onSetCback = nullptr, vector <string> *_strings = NULL):
		Property(VALUE_TYPE_ENUM, mode),
		onSetCback(onSetCback),
		value(0)
	{
		if (_strings)
		{
			strings = *_strings;
		}
	};
	virtual ~EnumProperty(){};
	void* get() override {
		return new int(value);
	}

	string toString() override {
		return strings.at(value);
	}
	vector <string> & getStrings() {return strings;}

	SetResult set(void* _value) override {
		SetResult ret = SET_OK;
		int val = *static_cast<int*>(_value);
		if(onSetCback)
		{
			ret = onSetCback(val);
		}
		if (ret == SET_OK)
		{
			value = val;
		}
		return ret;
	}

private:
	onSet onSetCback;
	int value;
	vector <string> strings;
};




