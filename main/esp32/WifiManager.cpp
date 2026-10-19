
#include "WifiManager.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"

static const char *TAG = "Wifi";

bool WifiManager::s_connected = false;

WifiManager::WifiManager():
	mode(WIFI_OFF),
	reconnect_timer(nullptr)
{
	wifi_config = {};
	ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(esp_netif_init());
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

	    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
	                                                        ESP_EVENT_ANY_ID,
	                                                        &sta_event_handler,
	                                                        this,
	                                                        &instance_any_id));
	    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
	                                                        IP_EVENT_STA_GOT_IP,
	                                                        &sta_event_handler,
	                                                        this,
	                                                        &instance_got_ip));
	    esp_netif_create_default_wifi_sta();
	    esp_netif_create_default_wifi_ap();

	esp_timer_create_args_t timer_args = {};
	timer_args.callback = &reconnect_timer_cb;
	timer_args.arg = this;
	timer_args.name = "wifi_reconnect";
	esp_timer_create(&timer_args, &reconnect_timer);
}

WifiManager::~WifiManager() {
	if (reconnect_timer) {
		esp_timer_stop(reconnect_timer);
		esp_timer_delete(reconnect_timer);
		reconnect_timer = nullptr;
	}
}

bool WifiManager::off(bool silent)
{
	s_connected = false;
	if (reconnect_timer) esp_timer_stop(reconnect_timer);
	mode = WIFI_OFF;
	esp_wifi_disconnect();
	esp_wifi_stop();
	esp_wifi_set_mode(WIFI_MODE_NULL);
	if (!silent) onMode(mode);
	return true;
};
bool WifiManager::join()
{
	s_connected = false;
	if (ssid.size())
	{
		mode = WIFI_CLI_JOIN;
		onMode(mode);
		s_retry_num = 0;
		wifi_init_sta(ssid,pwd);
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
	s_connected = false;
	if (reconnect_timer) esp_timer_stop(reconnect_timer);
	mode = WIFI_AP;
	uint8_t baseMac[6];
	esp_wifi_get_mac(WIFI_IF_STA, baseMac);
	char tmp[128];
	snprintf(tmp,sizeof(tmp),"%s_%02X:%02X:%02X:%02X:%02X:%02X",
			apHdr.c_str(),baseMac[0],baseMac[1],baseMac[2],baseMac[3],baseMac[4],baseMac[5]);

	init_ap(string(tmp));
	ip="192.168.4.1";
	onMode(mode);
	return true;
};

void WifiManager::wifi_init_sta(const string ssid, const string pwd)
{
	wifi_mode_t cur_mode;
	if (esp_wifi_get_mode(&cur_mode) == ESP_OK && cur_mode != WIFI_MODE_NULL) {
		esp_wifi_stop();
	}
	memset(&wifi_config, 0, sizeof(wifi_config));
	strncpy((char*)wifi_config.sta.ssid, ssid.c_str(), sizeof(wifi_config.sta.ssid) - 1);
	strncpy((char*)wifi_config.sta.password, pwd.c_str(), sizeof(wifi_config.sta.password) - 1);
	wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
	ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
	ESP_ERROR_CHECK(esp_wifi_start());

	ESP_LOGI(TAG, "wifi_init_sta finished.");
}

void WifiManager::init_ap(const string ssid)
{
	wifi_mode_t cur_mode;
	if (esp_wifi_get_mode(&cur_mode) == ESP_OK && cur_mode != WIFI_MODE_NULL) {
		esp_wifi_stop();
	}
	memset(&wifi_config, 0, sizeof(wifi_config));
	strncpy((char*)wifi_config.ap.ssid, ssid.c_str(), sizeof(wifi_config.ap.ssid) - 1);
	wifi_config.ap.ssid_len = strlen((char*)wifi_config.ap.ssid);
	wifi_config.ap.max_connection = 2;
	wifi_config.ap.authmode = WIFI_AUTH_OPEN;
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
	ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
	ESP_ERROR_CHECK(esp_wifi_start());
	ESP_LOGI(TAG, "init_ap finished.");
}

void WifiManager::reconnect_timer_cb(void* arg)
{
	WifiManager * me = (WifiManager *) arg;
	if (me->mode != WIFI_AP && me->mode != WIFI_OFF)
	{
		me->s_retry_num++;
		ESP_LOGI(TAG, "Tentativo di riconnessione Wi-Fi (#%u)...", (unsigned int)me->s_retry_num);
		esp_wifi_connect();
	}
}

void WifiManager::sta_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
	WifiManager * me = (WifiManager *) arg;
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        s_connected = false;
        if (me->mode != WIFI_AP && me->mode != WIFI_OFF) {
            ESP_LOGW(TAG, "Wi-Fi non connesso. Nuovo tentativo tra 3 secondi...");
            me->mode = WIFI_CLI_JOIN;
            if (me->reconnect_timer) {
                esp_timer_stop(me->reconnect_timer);
                esp_timer_start_once(me->reconnect_timer, 3000000); // 3 secondi
            }
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        if (me->reconnect_timer) {
            esp_timer_stop(me->reconnect_timer);
        }
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        char tmp[64];
        sprintf(tmp,IPSTR, IP2STR(&event->ip_info.ip));
        me->ip = string(tmp);
        me->s_retry_num = 0;
        s_connected = true;
        me->mode = WIFI_CLI_OK;
    	me->onMode(me->mode);
    }
}
