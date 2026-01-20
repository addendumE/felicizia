/*
 * ThingSpeak.cpp
 *
 *  Created on: 31 ott 2024
 *      Author: maurizio
 */

#include "ThingSpeak.h"
#include "esp_log.h"

const char *TAG = "TS";

ThingSpeak::ThingSpeak() {
}

ThingSpeak::~ThingSpeak() {
}


int ThingSpeak::publish(const string key,const Channel &ch, string &res)
{
	string payload = "https://api.thingspeak.com/update?api_key="+ key;
	for (auto i=0; i < ch.size(); i++)
	{
		payload += "&field" + to_string(i+1) + "=" + ch.at(i);
	}
	ESP_LOGI(TAG,"URL:%s",payload.c_str());
	string resp;
	int ret = getSynch(payload, resp, 443);
	ESP_LOGI(TAG,"RESP:%d",ret);
	res="["+to_string(ret)+"] "+ getHeaders()["Date"];
	return ret;
}



