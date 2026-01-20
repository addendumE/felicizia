
#include "WifiManager.h"
#include "esp_log.h"

static const char *TAG = "wifi";


WifiManager::WifiManager():
	Thread("Wifi"),
	mode(WIFI_OFF)
{


}

WifiManager::~WifiManager() {
}


void WifiManager::loop() {
}


bool WifiManager::off(bool silent)
{
	mode = WIFI_OFF;
	if (!silent)onMode(mode);
	return true;
};
bool WifiManager::join()
{
	if (ssid.size())
	{
		mode = WIFI_CLI_OK;
		onMode(mode);
		return true;
	}
	else
	{
		mode = WIFI_OFF;
		onMode(mode);
		return false;
	}
};

bool WifiManager::ap()
{
	mode = WIFI_AP;
	onMode(mode);
	return true;
};
