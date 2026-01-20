#include "Lock.h"
#include <stdio.h>

Lock::Lock() {
	pthread_mutexattr_t Attr;
	pthread_mutexattr_init(&Attr);
	//pthread_mutexattr_settype(&Attr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&plock, &Attr);
}

Lock::~Lock() {
	pthread_mutex_destroy(&plock);
}

void Lock::take(void)
{
	pthread_mutex_lock(&plock);
}

void Lock::give(void)
{
	pthread_mutex_unlock(&plock);
}
