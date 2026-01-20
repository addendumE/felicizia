#pragma once

#include "Thread.h"
#include <string>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

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
	string getIp(){return ip;};
	WifiManager();
	virtual ~WifiManager();
	string ssid;
	string pwd;
	string apHdr;
	virtual void onMode(Mode) = 0;
private:
	string ip;
	Mode mode;
	size_t s_retry_num;
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    wifi_config_t wifi_config;

	void loop();
	void wifi_init_sta(const string, const string);
	void init_ap(const string ssid);

	static void sta_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);

};
