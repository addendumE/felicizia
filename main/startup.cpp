#include "MainApp.h"
#include "esp_log.h"
MainApp *app;
extern "C" void startup() {
	app = new MainApp;
	app->start();
}