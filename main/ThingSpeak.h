#pragma once
#include "Http.h"

using namespace std;

class ThingSpeak: private Http {
public:
	ThingSpeak();
	virtual ~ThingSpeak();
	int publish(int channelID, string &value);
	void setKey(string k);
private:
	string key;
	string lastRes;
};

