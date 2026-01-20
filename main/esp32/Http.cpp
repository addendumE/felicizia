#include "Http.h"
static const char *TAG = "Http";

Http::Http():
		resp(NULL)
{
	// TODO Auto-generated constructor stub

}

Http::~Http() {
	// TODO Auto-generated destructor stub
}


int Http::getSynch(const std::string url, std::string &_resp,int port)
{
	esp_http_client_config_t config={};
	int http_status = -1;
	esp_err_t err;
	resp = &_resp;
	config.url = url.c_str();
	config.port = port;
	config.auth_type = HTTP_AUTH_TYPE_NONE;
	config.event_handler = Http_EventHandler;
	config.user_data = (void*)this;
	config.timeout_ms = 10000;
	config.skip_cert_common_name_check = true;
    config.transport_type = HTTP_TRANSPORT_OVER_TCP;

    client = esp_http_client_init (&config);
	printf ("\n************** %lu ***************\n",esp_get_free_heap_size());
    if (client == NULL)
    {
    	ESP_LOGE (TAG,"Errore creazione client");
        return http_status;
    }
    esp_http_client_set_method (client, HTTP_METHOD_GET);
    if (ESP_OK != esp_http_client_set_header (client, "User-Agent", "ESP32"))
    {
    	ESP_LOGE (TAG,"Errore User-Agent");
    }

    if (ESP_OK != esp_http_client_set_header (client, "Connection", "close"))
    {
    	ESP_LOGE (TAG,"Errore Connection");
    }

    if ((err = esp_http_client_perform(client)) == ESP_OK)
    {
    	http_status  = esp_http_client_get_status_code(client);         /* Estrae http status dal client */
    	ESP_LOGI(TAG,"HTTP Status = %d",  http_status);
        esp_http_client_close(client);

    }
    else
    {
        ESP_LOGE(TAG,"HTTP request failed: %s", esp_err_to_name(err));
    }
    esp_http_client_cleanup(client);
    return http_status;
}


esp_err_t Http::Http_EventHandler(esp_http_client_event_t *evt)
{
	Http * me = (Http *) evt->user_data;

    switch(evt->event_id)                               /* Analizza evento http*/
    {
    case HTTP_EVENT_ERROR:
        ESP_LOGE(TAG,"HTTP_EVENT_ERROR");
        break;
    case HTTP_EVENT_ON_CONNECTED:
        ESP_LOGI(TAG,"HTTP_EVENT_ON_CONNECTED");
        break;
    case HTTP_EVENT_HEADER_SENT:
        break;
    case HTTP_EVENT_ON_HEADER:
        ESP_LOGI(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
        me->headers[string(evt->header_key)]=string(evt->header_value);
        break;
    case HTTP_EVENT_ON_DATA:                                        /* Ricevuti dati Http*/
    	*me->resp += string((char*)evt->data,(char*)evt->data + evt->data_len);
        break;
    case HTTP_EVENT_ON_FINISH:                                      /* Terminato evento http*/
        ESP_LOGI(TAG,"HTTP_EVENT_ON_FINISH");
        break;
    case HTTP_EVENT_DISCONNECTED:                                   /* Disconnessione http*/
        ESP_LOGI(TAG,"HTTP_EVENT_DISCONNECTED");
        break;
    default:
        ESP_LOGW(TAG,"unhandled event %d",evt->event_id);
        break;
    }
    return ESP_OK;
}


