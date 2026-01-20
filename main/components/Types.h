#pragma once
#include <map>
#include <string>
using namespace std;
namespace Types
{
	enum Type
	{
		TYPE_DEVICE,
		TYPE_ANALOG_INPUT,
		TYPE_ANALOG_VALUE,
		TYPE_BINARY_INPUT,
		TYPE_BINARY_VALUE,
		TYPE_BINARY_OUTPUT,
		TYPE_THRESHOLD
	};

	enum Unit
	{
		UNIT_NONE,
		UNIT_TEMPERATURE,
		UNIT_VOLTAGE,
		UNIT_DISTANCE,
		UNIT_PERCENT,
		UNIT_CONDUCTIVITY,
		UNIT_HOURS,
		UNIT_MINUTES
	};

	enum ValueType
	{
		VALUE_TYPE_BOOL,
		VALUE_TYPE_FLOAT,
		VALUE_TYPE_INTEGER,
		VALUE_TYPE_STRING,
		VALUE_TYPE_ENUM
	};

	enum PropertyId
	{
		PROP_NAME,
		PROP_TYPE,
		PROP_DESC,
		PROP_VALUE,
		PROP_OVERRIDE_VALUE,
		PROP_OVERRIDE,
		PROP_VALUE_STRING,
		PROP_DECIMALS,
		PROP_CAL_OFFSET,
		PROP_CAL_GAIN,
		PROP_FAIL,
		PROP_FREE_HEAP,
		PROP_DATE,
		PROP_TIME,
		PROP_TH_L,
		PROP_TH_H,
		PROP_TH_MODE,
		PROP_TH_LAST_VALUE
	};

extern const map<Type,string> typeNames;
extern const map<PropertyId,string> propertyNames;
extern const map<Unit,string> unitNames;
}
