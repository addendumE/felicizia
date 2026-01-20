/*
 * Timer.cpp
 *
 *  Created on: 15 set 2022
 *      Author: maurizio
 */

#include "Timer.h"
#include "esp_log.h"

#define TAG "timer"

Timer::Timer(const string &name, size_t delayms, bool periodic):
	delayms(delayms)
{
	timerHnd = xTimerCreate(name.c_str(), pdMS_TO_TICKS(delayms), periodic, ( void * )this, &timer_end);
}

Timer::~Timer() {
	xTimerDelete(timerHnd,portMAX_DELAY);
}


bool Timer::start(void) {
	return xTimerStart(timerHnd,portMAX_DELAY) == pdPASS;
}

bool Timer::changePeriod(size_t delayms)
{
	return xTimerChangePeriod(timerHnd,pdMS_TO_TICKS(delayms),portMAX_DELAY) == pdPASS;
}

void Timer::stop()
{
	xTimerStop(timerHnd,portMAX_DELAY);
}


void Timer::timer_end(TimerHandle_t xTimer) {
	Timer * me = (Timer*) pvTimerGetTimerID( xTimer );

	if (me->onTimerEndCback) me->onTimerEndCback();
}
