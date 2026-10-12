#include <Property.h>

Property::Property(ValueType type, Mode mode):
		type(type),
		mode(mode)
	{

	};
Property::~Property()
	{
	};

ValueType Property::getType(){return type;};
	bool Property::getPersistance(){return mode == MODE_WRITABLE_PERSISTENT;};
	bool Property::getWritable() {return mode != MODE_READONLY;};


StringProperty::StringProperty(Mode mode, onSet onSetCback):
		Property(VALUE_TYPE_STRING,mode),
		onSetCback(onSetCback)
{

};
StringProperty::~StringProperty()
{

};
string StringProperty::get()
{
	return value;
	}
	
	string StringProperty::toString(bool raw) {
			return value;
	}

	Property::SetResult StringProperty::set(const string &val)
	{
		SetResult ret = SET_OK;
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



FloatProperty::FloatProperty(Mode mode, Unit unit, int decimals,onSet onSetCback):
		Property(VALUE_TYPE_FLOAT, mode),
		onSetCback(onSetCback),
		unit(unit),
		decimals(decimals),
		value(0.0f)
	{};
	FloatProperty::~FloatProperty(){};
	float FloatProperty::get()
	{
		return value;	
	}

	string FloatProperty::toString(bool raw) {
		char buff[128];
		if (raw)
		{
			snprintf(buff,sizeof(buff),"%f",value);
		}
		else
		{
			snprintf(buff,sizeof(buff),"%.*f %s",decimals,value,getUnitName(unit));

			
		}
		return string(buff);
	}

	Property::SetResult FloatProperty::set(float &_val)
	{
		Property::SetResult ret = SET_OK;
		if(onSetCback)
		{
			ret =  onSetCback(_val);
		}
		if (ret == SET_OK)
		{
			value = _val;
		}
		return ret;
	}

IntProperty::IntProperty(Mode mode, onSet onSetCback):
		Property(VALUE_TYPE_INTEGER, mode),
		onSetCback(onSetCback),
		value(0)
	{};
IntProperty::~IntProperty(){};
	int IntProperty::get(){
		return value;
	}

	string IntProperty::toString(bool raw) {
		return to_string(value);
	}


	Property::SetResult IntProperty::set(int _value) {
		Property::SetResult ret = SET_OK;
		
		if(onSetCback)
		{
			ret = onSetCback(_value);
		}
		if (ret == SET_OK)
		{
			value = _value;
		}
		return ret;
	}


BoolProperty::BoolProperty(Mode mode, onSet onSetCback, string trues, string falses):
		Property(VALUE_TYPE_BOOL, mode),
		onSetCback(onSetCback),
		trues(trues),
		falses(falses),
		value(false)
	{};
	BoolProperty::~BoolProperty(){};
	bool BoolProperty::get() {
		return value; 
	}

	string BoolProperty::toString(bool raw) {
		if (raw)
			return (value) ? "true":"false";
		else
			return (value) ? trues:falses;
	}

	Property::SetResult BoolProperty::set(bool _value) {
		Property::SetResult ret = SET_OK;
		
		if(onSetCback)
		{
			ret = onSetCback(_value);
		}
		if (ret == SET_OK)
		{
			value = _value;
		}
		return ret;
	}


EnumProperty::EnumProperty(Mode mode, onSet onSetCback, vector <string> *_strings):
		Property(VALUE_TYPE_ENUM, mode),
		onSetCback(onSetCback),
		value(0)
	{
		if (_strings)
		{
			strings = *_strings;
		}
	};
EnumProperty::~EnumProperty(){};
	int EnumProperty::get() {
		return value;
	}

	string EnumProperty::toString(bool raw) {
		if (raw)
			return(to_string(value));
		else if (value >= 0 && static_cast<size_t>(value) < strings.size())
			return strings[value];
		else
			return to_string(value);
	}
	vector <string> & EnumProperty::getStrings() {return strings;}

	Property::SetResult EnumProperty::set(int _value) {
		if (!strings.empty() && (_value < 0 || static_cast<size_t>(_value) >= strings.size()))
		{
			return SET_RANGE_ERROR;
		}
		Property::SetResult ret = SET_OK;
		if(onSetCback)
		{
			ret = onSetCback(_value);
		}
		if (ret == SET_OK)
		{
			value = _value;
		}
		return ret;
	}



