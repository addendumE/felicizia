#include "MainApp.h"
#include "Persistance.h"
#include "esp_log.h"
static const char * TAG="APP";

#include "esp_system.h"
#include "esp_sntp.h"


MainApp::MainApp():
	reboot("reb", 1000),
	nvs("APP",NVS_READWRITE),
	objManager(new ObjManager()),
	mqtt(new MqttClient()),
	protocol(new MyProtocol(*objManager)),
	persistance(new MyPersistence(*protocol,*objManager,nvs,ts,*mqtt,*this)),
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
	#ifndef LINUX_PLATFORM
			esp_restart();
	#endif
		});

    dataManager->onBootButton([this]() {
		if (WifiManager::getMode()!=WifiManager::WIFI_AP)
		{
			ESP_LOGI(TAG,"activating AP from button");
			ap();
		}
        
    });


	WifiManager::ssid = dataManager->getSSID();
	WifiManager::pwd = dataManager->getPwd();
	WifiManager::apHdr = "felicizia";
	persistance->setUid(dataManager->getUid());
	ESP_LOGI(TAG,"UID:%s",dataManager->getUid().c_str());
	ESP_LOGI(TAG,"SSID:%s",WifiManager::ssid.c_str());
	ESP_LOGI(TAG,"MQTT:%s",dataManager->getUid().c_str());

	if (ssid.size())
	{
		ESP_LOGI(TAG,"joining %s",WifiManager::ssid.c_str());
		join();
	}
	else
	{
		ap();
	}
	ESP_LOGI(TAG,"starting WEB server");
	Websocket::start(80);

}

void MainApp::onMessage(const string& msg)
{
	string resp = protocol->onMessage(msg);
	Websocket::send(resp);
}

void MainApp::startMqtt()
{

		mqtt->setOnMessageCallback([&](const std::string& topic, const std::string& payload)
		{
			ESP_LOGI(TAG,"mqtt msg %s %s",topic.c_str(),payload.c_str());
			string resp = protocol->onMessage(payload);
			ESP_LOGI("PER","mqtt msg %s %s",payload.c_str(),resp.c_str());
			mqtt->publish(dataManager->getUid()+"/Uplink",resp);
		});
	mqtt->setOnConnectCallback([&]()
		{
			ESP_LOGI(TAG,"mqtt connected");
			mqtt->subscribe(dataManager->getUid()+"/Downlink");
			
		});
	mqtt->setOnDisconnectCallback([&]()
		{
			ESP_LOGI(TAG,"mqtt disconnected");
		});
		
	ESP_LOGI(TAG,"Tentativo di connessione al broker MQTT: %s", dataManager->getMqttUri().c_str());
	mqtt->start(dataManager->getMqttUri());
}

void MainApp::onMode(WifiManager::Mode mode)
{
	ESP_LOGI(TAG,"wifi mode now is: %d",mode);
	if (mode == WifiManager::WIFI_CLI_OK)
	{
#ifndef LINUX_PLATFORM
		ESP_LOGI(TAG, "Initializing SNTP");
		esp_sntp_setoperatingmode(ESP_SNTP_OPMODE_POLL);
		esp_sntp_setservername(0, "pool.ntp.org");
		esp_sntp_init();
		setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1);
		tzset();
#endif
		startMqtt();
	}
	if (mode == WifiManager::WIFI_CLI_FAIL)
	{
		vTaskDelay(pdMS_TO_TICKS(1000));
		join();
	}
}
