
#include "WifiManager.h"
#include "esp_log.h"

static const char *TAG = "wifi";

bool WifiManager::s_connected = false;

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
	s_connected = false;
	mode = WIFI_OFF;
	if (!silent)onMode(mode);
	return true;
};
bool WifiManager::join()
{
	if (ssid.size())
	{
		s_connected = true;
		mode = WIFI_CLI_OK;
		onMode(mode);
		return true;
	}
	else
	{
		s_connected = false;
		mode = WIFI_OFF;
		onMode(mode);
		return false;
	}
};

bool WifiManager::ap()
{
	s_connected = false;
	mode = WIFI_AP;
	onMode(mode);
	return true;
};
