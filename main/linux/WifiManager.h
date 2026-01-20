#pragma once
#include "Thread.h"
#include <string>
using namespace std;

class WifiManager: public Thread  {
public:
	enum Mode
		{
			WIFI_OFF,
			WIFI_AP,
			WIFI_CLI_JOIN,
			WIFI_CLI_OK,
			WIFI_CLI_FAIL
		};
	Mode getMode(){return mode;}
	bool off(bool silent=false);
	bool join();
	bool ap();
	string getIp(){return "192.168.123.100";};
	string ssid;
	string pwd;
	string apHdr;
	WifiManager();
	virtual ~WifiManager();
	virtual void onMode(Mode) = 0;
private:
	Mode mode;
	void loop();
};
