/*
 * Timer.cpp
 *
 *  Created on: 15 set 2022
 *      Author: maurizio
 */

#include "Timer.h"
#include "esp_log.h"

static const char *TAG = "Timer";

Timer::Timer(string name, size_t delayms, bool periodic):
	name(name),
	delayms(delayms),
	periodic(periodic),
	timerId{}
{
	struct sigevent sev = {};
	sev.sigev_notify = SIGEV_THREAD;
	sev.sigev_notify_function  = &end_thread;
	sev.sigev_value.sival_ptr = this;
	timer_create(CLOCK_MONOTONIC, &sev,&timerId);
}

Timer::~Timer() {
	timer_delete(timerId);
}


bool Timer::start() {
    struct itimerspec its = {};
	if (periodic)
	{
		its.it_value.tv_sec     = (delayms*1000) / 1000000;
		its.it_value.tv_nsec    = (delayms*1000) % 1000000;
		its.it_interval.tv_sec  = its.it_value.tv_sec;
		its.it_interval.tv_nsec = its.it_value.tv_nsec;
	}
	else
	{
		its.it_value.tv_sec = (delayms*1000) / 1000000;
		its.it_value.tv_nsec = (delayms*1000) % 1000000;
	}
	ESP_LOGI(TAG,"%s started %d",name.c_str(),delayms);
    return timer_settime(timerId, 0, &its, NULL) == 0;
}

void Timer::stop()
{
    struct itimerspec its;
	its.it_value.tv_sec = 0;
	its.it_value.tv_nsec = 0;
	its.it_interval.tv_sec = 0;
	its.it_interval.tv_nsec = 0;
	timer_settime(timerId, 0, &its, NULL);
	ESP_LOGI(TAG,"%s stopped",name.c_str());

}


bool Timer::changePeriod(size_t _delayms)
{
	ESP_LOGI(TAG,"%s change to %d",name.c_str(),_delayms);

	delayms = _delayms;
	start();
	return true;
}


void Timer::end_thread(union  sigval args) {
	Timer * me = (Timer*) args.sival_ptr;
	ESP_LOGI(TAG,"%s expired",me->name.c_str());
	if (me->onTimerEndCback) me->onTimerEndCback();
}
