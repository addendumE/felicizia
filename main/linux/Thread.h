#pragma once

#include <pthread.h>
#include <unistd.h>

#include <string>

class Thread {
public:
	Thread(std::string);
	virtual ~Thread();
	bool start();
	void stop();
	void pause(int);
	void cancel();
protected:
	static void msleep(int x) { usleep(x*1000);}
private:
	virtual void loop(void) = 0;
	std::string id;
	pthread_t tid;
	static void * thread_loop(void *);
};
