#include "MainApp.h"
#include "Persistance.h"
#include "esp_log.h"
static const char * TAG="APP";

#ifndef LINUX
#include "esp_system.h"
#endif


MainApp::MainApp():
	reboot("reb", 1000),
	nvs("APP",NVS_READWRITE),
	objManager(new (ObjManager)),
	protocol(new Protocol(*objManager)),
	persistance(new MyPersistence(*protocol,*objManager,nvs,ts)),
	dataManager(new DataManager(*objManager,*persistance,hal))
{
}

MainApp::~MainApp()
{

}


void MainApp::start()
{
	reboot.onExpired([&]()
		{
			ESP_LOGI(TAG,"rebooting!!!");
	#ifndef LINUX
			esp_restart();
	#endif
		});

	//string ssid,pwd,mode,key;
	//nvs.getString("ssid",ssid);
	//nvs.getString("pwd",pwd);
	//nvs.getString("pwd",key);

	ts.setKey(dataManager->getKey());
	WifiManager::ssid = dataManager->getSSID();
	WifiManager::pwd = dataManager->getPwd();
	WifiManager::apHdr = "felicizia";


	if (ssid.size())
	{
		join();
	}
	else
	{
		ap();
	}
	ESP_LOGI(TAG,"starting WEB server");
	protocol->start(80);
}


void MainApp::onMode(WifiManager::Mode mode)
{

}

