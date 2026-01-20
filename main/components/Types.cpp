#include "Types.h"
using namespace Types;

const map<Unit,string> Types::unitNames = {
   {UNIT_NONE, "-"},
   {UNIT_TEMPERATURE, "°C"},
   {UNIT_VOLTAGE, "V"},
   {UNIT_DISTANCE, "m"},
   {UNIT_PERCENT, "%"},
   {UNIT_CONDUCTIVITY,"uS/cm"},
   {UNIT_HOURS,"h"},
   {UNIT_MINUTES,"min"}
};

const map<PropertyId,string> Types::propertyNames = {
   {PROP_NAME, "name"},
   {PROP_TYPE, "type"},
   {PROP_DESC, "description"},
   {PROP_VALUE, "value"},
   {PROP_OVERRIDE_VALUE, "override_value"},
   {PROP_OVERRIDE, "override"},
   {PROP_VALUE_STRING,"value_string"},
   {PROP_DECIMALS, "decimals"},
   {PROP_CAL_OFFSET, "cal_offset"},
   {PROP_CAL_GAIN, "cal_gain"},
   {PROP_FAIL,"fail"},
   {PROP_FREE_HEAP,"free_heap"},
   {PROP_TH_L,"low_level"},
   {PROP_TH_H,"high_level"},
   {PROP_TH_MODE,"mode"},
   {PROP_TH_LAST_VALUE,"last_value"},
};

const map<Type,string> Types::typeNames = {
   {TYPE_DEVICE, "DEV"},
   {TYPE_ANALOG_INPUT, "AI"},
   {TYPE_ANALOG_VALUE, "AV"},
   {TYPE_BINARY_INPUT, "BI"},
   {TYPE_BINARY_VALUE, "BV"},
   {TYPE_BINARY_OUTPUT, "BO"},
   {TYPE_THRESHOLD, "TH"}
};
