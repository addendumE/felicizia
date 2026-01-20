#pragma once

#include <pthread.h>

class Lock {
public:
	Lock();
	virtual ~Lock();
	void take();
	void give();
private:
	pthread_mutex_t plock;
};
