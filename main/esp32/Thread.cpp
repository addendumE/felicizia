/*
 * Task.cpp
 *
 *  Created on: 20 dic 2023
 *      Author: maurizio
 */

#include "Thread.h"
#include "esp_log.h"

#define TAG "Thread"

Thread::Thread(const string name):
	name(name),
	handle(NULL)
{
	ESP_LOGI(TAG,"created");
}

bool Thread::start()
{
	ESP_LOGI(TAG,"start");

	bool ret = false;
	if (handle == NULL)
	{
        ESP_LOGI(TAG,"start task %s",name.c_str());
		ret = xTaskCreate(taskHandle,name.c_str(),8192,this,10,&handle) == pdPASS;
	}
	else
	{
        ESP_LOGE(TAG,"can't start task %s because already runnig",name.c_str());

	}
	return ret;
}

void Thread::stop()
{
	if (handle != NULL)
	{
		vTaskDelete(handle);
		handle = NULL;
	}
}

void Thread::cancel()
{
	stop();
}

Thread::~Thread() {
	stop();
}

void Thread::taskHandle( void * pvParameters )
{
	((Thread *) pvParameters)->loop();
    vTaskDelete( NULL );
}

void Thread::msleep(int x) {
	vTaskDelay( pdMS_TO_TICKS(x));
}
