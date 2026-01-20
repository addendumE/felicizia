#pragma once
#include <stdlib.h>
class OneWire {
public:
	OneWire() {}
	virtual ~OneWire() {}
	bool init(int){return true;}
	bool readTemperature(int i, float &res){res = 10.0f+(20.0f*rand()/RAND_MAX);return true;}
};
