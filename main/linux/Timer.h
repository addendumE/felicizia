/*
 * Timer.h
 *
 *  Created on: 15 set 2022
 *      Author: maurizio
 */

#ifndef TIMER_H_
#define TIMER_H_
#include <pthread.h>
#include <string>
#include <functional>
#include <time.h>
#include <signal.h>

using namespace std;

class Timer {
public:
	typedef function <void()> timerEndCback;
	Timer(string name, size_t delayms, bool periodic = false);
	virtual ~Timer();
	void onExpired(timerEndCback cback) {onTimerEndCback = cback;};
	bool start();
	void stop();
	bool changePeriod(size_t delayms);

private:
	string name;
	timer_t	timerId;
	size_t delayms;
	bool periodic;
	timerEndCback onTimerEndCback;
	static void end_thread(union  sigval );
};

#endif /* TIMER_H_ */
