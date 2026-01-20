#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

class Lock {
public:
	Lock();
	virtual ~Lock();
	void take();
	void give();
private:
	SemaphoreHandle_t xSemaphore;
};
