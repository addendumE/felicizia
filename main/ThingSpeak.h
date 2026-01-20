#pragma once
#include "Http.h"

using namespace std;

typedef vector <string> Channel;


class ThingSpeak: private Http {
public:
	ThingSpeak();
	virtual ~ThingSpeak();
	int publish(const string key,const Channel &,string &res);
};

