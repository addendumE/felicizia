#pragma once

#include <esp_system.h>
#include <esp_log.h>
#include <esp_http_client.h>
#include <stdint.h>
#include <functional>
#include <vector>
#include <string>
#include <map>
using namespace std;

class Http {
public:
	Http();
	virtual ~Http();
	int getSynch(const string url, string &resp, int port = 80);
	map <string,string> &getHeaders(){return headers;}
private:
	string *resp;
	map <string,string> headers;
    esp_http_client_handle_t client;
    static esp_err_t Http_EventHandler(esp_http_client_event_t *evt);
};
