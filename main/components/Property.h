#pragma once
#include "Types.h"
#include <functional>
#include <sstream>
#include <iomanip>
#include <vector>
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
		SET_STORE_ERROR,
		SET_WRONG_TYPE
	};

	Property(ValueType type, Mode mode);
	virtual ~Property();
	
	virtual string toString(bool raw = false) = 0;
	ValueType getType();
	bool getPersistance();
	bool getWritable();
protected:
	string value;
private:
	ValueType type;
	Mode mode;

};

class StringProperty:public Property
{
public:
	using onSet = function <SetResult(string)>;
	StringProperty(Mode mode = MODE_READONLY, onSet onSetCback = nullptr);
	virtual ~StringProperty();
	string get();
	string toString(bool raw);
	SetResult set(const string &val);
private:
	onSet onSetCback;
};


class FloatProperty:public Property
{
public:
	using onSet = function <SetResult(float)>;
	FloatProperty(Mode mode = MODE_READONLY, Unit unit = UNIT_NONE , int decimals = 2,onSet onSetCback = nullptr);
	virtual ~FloatProperty();
	float get();
	string toString(bool raw);
	SetResult set(float &_val);
private:
	onSet onSetCback;
	Unit unit;
	int decimals;
};

class IntProperty:public Property
{
public:
	using onSet = function <SetResult(int)>;
	IntProperty(Mode mode = MODE_READONLY, onSet onSetCback = nullptr);
	virtual ~IntProperty();
	int get();
	string toString(bool raw);
	SetResult set(int _value);
private:
	onSet onSetCback;
};

class BoolProperty:public Property
{
public:
	using onSet = function <SetResult(bool)>;
	BoolProperty(Mode mode = MODE_READONLY, onSet onSetCback = nullptr, string trues="true", string falses="false");
	virtual ~BoolProperty();
	bool get();
	string toString(bool raw);
	SetResult set(bool _value);

private:
	onSet onSetCback;
	string trues;
	string falses;
};

class EnumProperty:public Property
{
public:
	using onSet = function <SetResult(int)>;
	EnumProperty(Mode mode = MODE_READONLY, onSet onSetCback = nullptr, vector <string> *_strings = NULL);
	virtual ~EnumProperty();
	int get();
	string toString(bool raw);
	vector <string> & getStrings();
	SetResult set(int _value);
private:
	onSet onSetCback;
	vector <string> strings;
};




