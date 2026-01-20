/*
 * Nvs.cpp
 *
 *  Created on: 20 dic 2023
 *      Author: maurizio
 */

#include "Nvs.h"
#include "esp_log.h"

static const char *TAG = "NVS";

Nvs::Nvs(const string ns,nvs_open_mode_t open_mode):
	ns(ns)
{
	Lock::take();
	esp_err_t res = nvs_open(ns.c_str(), open_mode, &handle);
	if (res != ESP_OK)
	{
		ESP_LOGW(TAG,"Nvs [%s] open error %x",ns.c_str(),(int)res);
		opened = false;
	}
	else
	{
		opened = true;
	}
	Lock::give();
}

bool Nvs::getString(const string key, string &value)
{
	Lock::take();
	bool ret = false;
	ESP_LOGI(TAG,"getString [%s]%s",ns.c_str(),key.c_str());
	if (opened)
	{
		size_t size;
		esp_err_t res = nvs_get_str(handle, key.c_str(), NULL, &size);
		if (res == ESP_OK)
		{
			char *data = (char*)malloc(size);
			if (data)
			{
				nvs_get_str(handle, key.c_str(), data, &size);
				value = string(data,size-1);
				ESP_LOGD(TAG,"getString value:%s",value.c_str());

				ret = true;
				free (data);
			}
			else
			{
				ESP_LOGE(TAG,"getString:MALLOC error");
			}
		}
		else
		{
		    ESP_LOGE(TAG,"getString: ERROR %x",ret);
		}
	}
	else
	{
	    ESP_LOGE(TAG," getString:NOT OPENED");
	}
	Lock::give();
	return ret;
}

bool Nvs::setString(const string key, const string value)
{
	Lock::take();
	bool ret = false;
	ESP_LOGI(TAG,"setString [%s]%s",ns.c_str(),key.c_str());
	ESP_LOGD(TAG,"setString value:%s",value.c_str());
	if (opened)
	{
		esp_err_t res = nvs_set_str(handle, key.c_str(), value.c_str());
		if (res == ESP_OK)
		{
			nvs_commit(handle);
			ret = true;
		}
		else
		{
			ESP_LOGE(TAG,"setString: error %x",res);
		}
	}
	else
	{
	    ESP_LOGE(TAG,"setString: NOT OPENED");

	}
	Lock::give();
	return ret;
}

bool Nvs::getNumber(const string key, uint32_t &value)
{
	Lock::take();
	bool ret = false;
	ESP_LOGI(TAG,"getNumber [%s]%s",ns.c_str(),key.c_str());
	if (opened)
	{
		uint32_t tmp;
		esp_err_t res = nvs_get_u32 (handle, key.c_str(), &tmp);
		if (res == ESP_OK)
		{
			value = tmp;
			ESP_LOGD(TAG,"getNumber value:%08" PRIx32 ,tmp);
			ret = true;
		}
		else
		{
		    ESP_LOGE(TAG,"getNumber: ERROR %x",ret);
		}
	}
	else
	{
	    ESP_LOGE(TAG," getNumber:NOT OPENED");
	}
	Lock::give();
	return ret;
}

vector <string> Nvs::getKeys()
{
	vector <string> ret;
	nvs_iterator_t it = NULL;
	esp_err_t res = nvs_entry_find_in_handle(handle, NVS_TYPE_ANY, &it);
	 while(res == ESP_OK) {
		 nvs_entry_info_t info;
	     nvs_entry_info(it, &info); // Can omit error check if parameters are guaranteed to be non-NULL
	     printf("key '%s', type '%d' \n", info.key, info.type);
	     ret.push_back(info.key);
	     res = nvs_entry_next(&it);
	 }
	 nvs_release_iterator(it);
	 return ret;
}


Nvs::~Nvs() {
	Lock::take();
	if (opened)
	{
		nvs_close(handle);
	}
	Lock::give();
}

