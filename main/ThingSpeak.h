#pragma once
#include "Http.h"
#include <map>
#include <esp_log.h>
using namespace std;
#include <string>
#include <sstream>
#include <iomanip>
#include <cctype>

class ThingSpeak: private Http, private map<size_t,string>  {
public:
	
	ThingSpeak();
	virtual ~ThingSpeak();
	void setValue(size_t idx, const string &_val);
	void setValue(size_t idx, float _val);
	void clean();
	int publish(const string &key);
private:
	std::string uriEscape(const std::string& input);
	string getString();
};

