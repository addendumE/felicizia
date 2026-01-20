#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string>
using namespace std;

class Thread {
public:
	Thread(const string name);
	bool start();
	void stop();
	void cancel();
	virtual ~Thread();
	static void msleep(int x);
private:
	string name;
	TaskHandle_t handle;
	virtual void loop(void) = 0;
	static void taskHandle( void * pvParameters );

};

