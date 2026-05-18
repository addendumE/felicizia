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
		decimals(decimals)
	{};
	FloatProperty::~FloatProperty(){};
	float FloatProperty::get()
	{
		if (value.empty())
        	return 0.0f;
		else
        	return std::stof(value);	
	}

	string FloatProperty::toString(bool raw) {
		float val = get();
		char buff[128];
		if (raw)
		{
			snprintf(buff,sizeof(buff),"%f",val);
		}
		else
		{
			snprintf(buff,sizeof(buff),"%.*f %s",decimals,val,getUnitName(unit));

			
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
			value = to_string(_val);
		}
		return ret;
	}

IntProperty::IntProperty(Mode mode, onSet onSetCback):
		Property(VALUE_TYPE_INTEGER, mode),
		onSetCback(onSetCback)
	{};
IntProperty::~IntProperty(){};
	int IntProperty::get(){
		if (value.empty())
        	return 0;
		else
        	return std::stoi(value);
	}

	string IntProperty::toString(bool raw) {
		return value;
	}


	Property::SetResult IntProperty::set(int _value) {
		Property::SetResult ret = SET_OK;
		
		if(onSetCback)
		{
			ret = onSetCback(_value);
		}
		if (ret == SET_OK)
		{
			value = to_string(_value);
		}
		return ret;
	}


BoolProperty::BoolProperty(Mode mode, onSet onSetCback, string trues, string falses):
		Property(VALUE_TYPE_BOOL, mode),
		onSetCback(onSetCback),
		trues(trues),
		falses(falses)
	{};
	BoolProperty::~BoolProperty(){};
	bool BoolProperty::get() {
		return (value=="true"); 
	}

	string BoolProperty::toString(bool raw) {
		if (raw)
			return (get()) ? "true":"false";
		else
			return (get()) ? trues:falses;
	}

	Property::SetResult BoolProperty::set(bool _value) {
		Property::SetResult ret = SET_OK;
		
		if(onSetCback)
		{
			ret = onSetCback(_value);
		}
		if (ret == SET_OK)
		{
			value = (_value)? "true":"false";
		}
		return ret;
	}


EnumProperty::EnumProperty(Mode mode, onSet onSetCback, vector <string> *_strings):
		Property(VALUE_TYPE_ENUM, mode),
		onSetCback(onSetCback)
	{
		if (_strings)
		{
			strings = *_strings;
		}
	};
EnumProperty::~EnumProperty(){};
	int EnumProperty::get() {
		if (value.empty()) return 0.0f;
		return std::stoi(value);
	}

	string EnumProperty::toString(bool raw) {
		if (raw)
			return(value);
		else
			return strings.at(get());
	}
	vector <string> & EnumProperty::getStrings() {return strings;}

	Property::SetResult EnumProperty::set(int _value) {
		Property::SetResult ret = SET_OK;
		if(onSetCback)
		{
			ret = onSetCback(_value);
		}
		if (ret == SET_OK)
		{
			value = to_string(_value);
		}
		return ret;
	}



