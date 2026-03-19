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

void ThingSpeak::setKey(string _key)
{
	key = _key;
}


int ThingSpeak::publish(int channelID, string &value)
{
	string payload = "https://api.thingspeak.com/update?api_key="+ key;
	payload += "&field" + to_string(channelID) + "=" + value;
	ESP_LOGI(TAG,"URL:%s",payload.c_str());
	string resp;
	int ret = getSynch(payload, resp, 443);
	ESP_LOGI(TAG,"RESP:%d",ret);
	lastRes="["+to_string(ret)+"] "+ getHeaders()["Date"];
	return ret;
}



