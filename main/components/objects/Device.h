#pragma once
#include "Base.h"

class Device : public Base {
public:
	Device(string id, string name,Persistance &p);
	virtual ~Device();
	void setFreeHeap(float);
private:
};
