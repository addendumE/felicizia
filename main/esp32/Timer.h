#pragma once


#include <string>
#include <functional>
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"

using namespace std;

class Timer {
public:
	typedef function <void()> timerEndCback;
	Timer(const string &name, size_t delayms, bool periodic = false);
	virtual ~Timer();
	bool changePeriod(size_t delayms);

	void onExpired(timerEndCback cback) {onTimerEndCback = cback;};
	bool start();
	void stop();

private:
	TimerHandle_t	timerHnd;
	timerEndCback onTimerEndCback;
	size_t delayms;
	static void timer_end( TimerHandle_t xTimer );

};
