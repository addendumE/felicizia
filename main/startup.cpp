#include "MainApp.h"

MainApp *app;
extern "C" void startup() {
	app = new MainApp;
	app->start();
}

#if 0
#include <Gcm.hpp>
extern "C" void startup() {
	Gcm mq;
	MqttClient
	std::vector<unsigned char> pl;
	std::string out;
	pl  = mq.encrypt("ciao");
	out = mq.decrypt(pl);
	printf("********** %s ********\n",out.c_str());
}
#endif