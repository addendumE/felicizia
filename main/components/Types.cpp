#include "Types.h"

namespace Types {

const TypeEntry typeEntries[] = {
	{TYPE_DEVICE, "DEV"},
	{TYPE_ANALOG_INPUT, "AI"},
	{TYPE_ANALOG_VALUE, "AV"},
	{TYPE_BINARY_INPUT, "BI"},
	{TYPE_BINARY_VALUE, "BV"},
	{TYPE_BINARY_OUTPUT, "BO"},
	{TYPE_THRESHOLD, "TH"}
};
const int typeEntriesCount = sizeof(typeEntries) / sizeof(TypeEntry);

const PropertyEntry propertyEntries[] = {
	{PROP_NAME, "name"},
	{PROP_TYPE, "type"},
	{PROP_DESC, "description"},
	{PROP_VALUE, "value"},
	{PROP_OVERRIDE_VALUE, "override_value"},
	{PROP_OVERRIDE, "override"},
	{PROP_VALUE_STRING, "value_string"},
	{PROP_DECIMALS, "decimals"},
	{PROP_CAL_OFFSET, "cal_offset"},
	{PROP_CAL_GAIN, "cal_gain"},
	{PROP_FAIL, "fail"},
	{PROP_FREE_HEAP, "free_heap"},
	{PROP_TH_L, "low_level"},
	{PROP_TH_H, "high_level"},
	{PROP_TH_MODE, "mode"},
	{PROP_TH_LAST_VALUE, "last_value"},
	{PROP_SSID, "ssid"},
	{PROP_SSID_KEY, "ssid_key"},
	{PROP_TS_KEY, "ts_key"},
	{PROP_SW_VERSION, "sw_version"},
	{PROP_FILTER, "filter"},
	{PROP_FEEDBACK, "feedback"}
};
const int propertyEntriesCount = sizeof(propertyEntries) / sizeof(PropertyEntry);

const char* unitNames[] = {
	"-",
	"°C",
	"V",
	"m",
	"%",
	"uS/cm",
	"h",
	"min",
	"s"
};
const int unitNamesCount = sizeof(unitNames) / sizeof(char*);

const char* getTypeName(Type type) {
	for (int i = 0; i < typeEntriesCount; ++i) {
		if (typeEntries[i].type == type) return typeEntries[i].name;
	}
	return "UNKNOWN";
}

const char* getPropertyName(PropertyId propId) {
	for (int i = 0; i < propertyEntriesCount; ++i) {
		if (propertyEntries[i].id == propId) return propertyEntries[i].name;
	}
	return "unknown";
}

const char* getUnitName(Unit unit) {
	if (unit >= 0 && unit < unitNamesCount) return unitNames[unit];
	return "-";
}

} // namespace Types
