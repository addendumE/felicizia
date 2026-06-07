#include "MainApp.h"
#include "Persistance.h"
#include "esp_log.h"
static const char * TAG="APP";

#include "esp_system.h"
#include "esp_sntp.h"


MainApp::MainApp():
	reboot("reb", 5000),
	otaConfirm("ota",20000),
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

	otaConfirm.onExpired([&]()
		{
	#ifndef LINUX_PLATFORM
			check_ota_state();
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
	otaConfirm.start();

}

void MainApp::onMessage(const string& msg)
{
	string resp = protocol->onMessage(msg);
	Websocket::send(resp);
}

void MainApp::startMqtt()
{

		mqtt->setOnMessageCallback([&](const std::string& topic, const char * payload, size_t datasize,size_t offset, size_t totLen)
		{
			ESP_LOGI(TAG,"OnMessageCallback topic %s %d %d %d",topic.c_str(),datasize,offset,totLen);
			if (topic==dataManager->getUid()+"/Firmware")
			{
#ifdef LINUX_PLATFORM
				otaStart();
				otaProcess(payload,datasize);
				otaEnd();
#else
				if (offset == 0) {
					dataManager->stop();
					otaStart();
				}	
				otaProcess(payload,datasize);
				if ((offset + datasize) == totLen) {
					otaEnd();
					dataManager->setDeviceStatus("REBOOT");
					reboot.start();
				}
#endif
			}
			else if (topic == dataManager->getUid() + "/Downlink")
			{
				static std::string downlinkBuffer;
				if (offset == 0) downlinkBuffer.clear();
				downlinkBuffer.append(payload, datasize);

				if (offset + datasize == totLen) {
					string resp = protocol->onMessage(downlinkBuffer);
					mqtt->publish(dataManager->getUid() + "/Uplink", resp);
					dataManager->setDeviceStatus("OK");
					downlinkBuffer.clear();
				}
			}
		});
	mqtt->setOnConnectCallback([&]()
		{
			ESP_LOGI(TAG,"mqtt connected");
			mqtt->subscribe(dataManager->getUid()+"/Downlink");
			mqtt->subscribe(dataManager->getUid()+"/Firmware");
			
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



esp_err_t MainApp::otaStart()
{
	esp_err_t err;
    ESP_LOGI(TAG, "Starting OTA");
    const esp_partition_t *configured = esp_ota_get_boot_partition();
    const esp_partition_t *running = esp_ota_get_running_partition();
    if(configured==NULL || running == NULL)
    {
        ESP_LOGE(TAG,"OTA data not found");
        return ESP_FAIL;
    }

    if (configured != running)
    {
        ESP_LOGW(TAG, "Configured OTA boot partition at offset 0x%08lx, but running from offset 0x%08lx",
                 configured->address, running->address);
        ESP_LOGW(TAG, "(This can happen if either the OTA boot data or preferred boot image become corrupted somehow.)");
    }
    ESP_LOGI(TAG, "Running partition type %d subtype %d (offset 0x%08lx)",
             running->type, running->subtype, running->address);

    ota_update_partition = esp_ota_get_next_update_partition(NULL);
    ESP_LOGI(TAG, "Writing to partition subtype %d at offset 0x%lx",
             ota_update_partition->subtype, ota_update_partition->address);

    err = esp_ota_begin(ota_update_partition, OTA_SIZE_UNKNOWN, &ota_update_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "esp_ota_begin failed ");
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "esp_ota_begin succeeded");
    return ESP_OK;	
}
esp_err_t MainApp::otaProcess(const char *data, size_t len)
{
	return esp_ota_write(ota_update_handle, (const void *)data, len);

}
esp_err_t MainApp::otaEnd()
{
	 esp_err_t err = esp_ota_end(ota_update_handle);
	 if (err != ESP_OK) {
		 if (err == ESP_ERR_OTA_VALIDATE_FAILED) {
			 ESP_LOGE(TAG, "Image validation failed, image is corrupted");
	     }
	     ESP_LOGE(TAG, "esp_ota_end failed (%s)!", esp_err_to_name(err));
	     return ESP_FAIL;
	 }
	 err = esp_ota_set_boot_partition(ota_update_partition);
	 if (err != ESP_OK) {
	     ESP_LOGE(TAG, "esp_ota_set_boot_partition failed (%s)!", esp_err_to_name(err));
	     return ESP_FAIL;
	 }
	 ESP_LOGI(TAG, "New boot partition set successfully");

	 return err;
}

void MainApp::check_ota_state(void)
{
    const esp_partition_t *running = esp_ota_get_running_partition();

    esp_ota_img_states_t ota_state;

    if (esp_ota_get_state_partition(running, &ota_state) == ESP_OK) {
        ESP_LOGI(TAG, "Firmware OTA status: %d",ota_state);
        if (ota_state == ESP_OTA_IMG_PENDING_VERIFY) {
            ESP_LOGI(TAG, "Marking app valid");
            esp_ota_mark_app_valid_cancel_rollback();
        }
		else
		{
			ESP_LOGI(TAG, "APP valid");
		}
    }
}