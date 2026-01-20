#include "MainApp.h"
#include "Persistance.h"
#include "esp_log.h"
static const char * TAG="APP";

#ifndef LINUX
#include "esp_system.h"
#endif


MainApp::MainApp():
	reboot("reb", 1000),
	publishTimer("pub", 3600,true),
	nvs("APP",NVS_READWRITE),
	objManager(new (ObjManager)),
	protocol(new Protocol(*objManager)),
	persistance(new MyPersistence(*protocol,*objManager,nvs)),
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

		publishTimer.onExpired([&]()
		{
			ESP_LOGI(TAG,"publish timer expired!!!");

			publishData();
		});


	string ssid,pwd,mode;
	nvs.getString("ssid",ssid);
	nvs.getString("pwd",pwd);

	WifiManager::ssid = ssid;
	WifiManager::pwd = pwd;
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


void MainApp::publishData()
{
	string key;
	nvs.getString("key",key);
	PublishData publishData;
	//"MU572UZGU2DB12GF"
	string res;
	ThingSpeak::publish(key,publishData,res);
	ESP_LOGI(TAG,"%s",res.c_str());
	nvs.setString("lastts",res);
}

void MainApp::onMode(WifiManager::Mode mode)
{

}

