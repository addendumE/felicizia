#include "Http.h"
#include <curl/curl.h>
#include <resolv.h>
#include <esp_log.h>

static const char *TAG = "HTTP";

Http::Http() {

}

Http::~Http() {
}


int Http::getSynch(const std::string url, std::string &resp, int port)
{
	headers.clear();
	long http_code = -1;
	//flush DNS cache otherwise Curl can fail name resolution
	//see: https://curl.se/libcurl/c/CURLOPT_DNS_CACHE_TIMEOUT.html
	res_init();
	CURL * curl = curl_easy_init();
	if (curl)
	{
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_data);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_PORT, port);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp);
		int curl_code = curl_easy_perform(curl);
		if (curl_code == CURLE_OK)
		{
		    curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_code);
		    if (http_code != 200)
		    	ESP_LOGW(TAG, "returned %d",http_code);
		}
		else
		{
	    	ESP_LOGE(TAG, "get error %d",curl_code);
		}
	    curl_easy_cleanup(curl);
	}
	return http_code;


}

int Http::get(const std::string url, uint64_t &contentLength, int port)
{
	long http_code = -1;
	contentLength = 0;
	res_init();
	CURL * curl = curl_easy_init();
	if (curl)
	{
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_data);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_PORT, port);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
		int curl_code = curl_easy_perform(curl);
		if (curl_code == CURLE_OK)
		{
		    curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_code);
		    if (http_code != 200)
		    	ESP_LOGW(TAG, "returned %d",http_code);
		}
		else
		{
	    	ESP_LOGE(TAG, "get error %d",curl_code);
		}
	    curl_easy_cleanup(curl);
	}
	return http_code;

}

size_t Http::curl_write_data(char *ptr, size_t size, size_t nmemb, void *userdata)
{
	std::string *stream = (std::string *) userdata;
    size_t count = size * nmemb;
    *stream += std::string(ptr,ptr+count);
    return count;
}
