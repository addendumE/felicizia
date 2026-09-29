#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string>
using namespace std;

class Thread {
public:
	Thread(const string name, size_t stack_size = 8192);
	bool start();
	void stop();
	void cancel();
	virtual ~Thread();
	static void msleep(int x);
private:
	string name;
	TaskHandle_t handle;
	size_t stack_size;
	virtual void loop(void) = 0;
	static void taskHandle( void * pvParameters );

};

