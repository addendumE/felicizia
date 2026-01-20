#pragma once
#include "soc/soc_caps.h"

#include "esp_log.h"

using namespace std;

class Hal{
public:
	Hal();
	virtual ~Hal();
	float heapOccupation();
	bool requestOutput(int idx, bool val =false);
	bool setOutput(int idx, bool val =false);

private:
};
