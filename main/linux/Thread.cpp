/*
 * thread.cpp
 *
 *  Created on: 6 apr 2021
 *      Author: maurizio
 */

#include "Thread.h"
#include <stdio.h>

Thread::Thread(std::string _id):
	id(_id),
	tid(0)
{
}

Thread::~Thread() {
	stop();
}

bool Thread::start()
{
	int ret = -1;
	if (tid == 0)
	{
		ret = pthread_create(&tid, NULL, thread_loop, this);
		if (ret != 0)
		{
			printf("Thread: create thread %s error\n",id.c_str());
		}
	}
	else
	{
		printf("Thread already running: can't start\n");
	}
	return (ret==0);
}

void Thread::stop()
{
	if (tid > 0)
	{
		pthread_join(tid,NULL);
		tid = 0;
	}
}

void Thread::pause(int ms)
{
	usleep(ms*1000);
}

void* Thread::thread_loop(void *arg)
{
	Thread * me = (Thread *) arg;
	me->loop();
	printf("Thread: exit %s\n",me->id.c_str());

	return NULL;
}


void Thread::cancel()
{
	pthread_cancel(tid);
}

