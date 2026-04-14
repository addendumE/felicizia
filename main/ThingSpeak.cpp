/*
 * ThingSpeak.cpp
 *
 *  Created on: 31 ott 2024
 *      Author: maurizio
 */

#include "ThingSpeak.h"
#include "esp_log.h"

const char *TAG = "TS";

ThingSpeak::ThingSpeak()
{

}
ThingSpeak::~ThingSpeak()
{

}
	void ThingSpeak::setValue(size_t idx, const string &_val)
	{
	     (*this)[idx] = uriEscape(_val);
	}
	void ThingSpeak::setValue(size_t idx, float _val)
	{
	    (*this)[idx] = to_string(_val);
	}
	void ThingSpeak::clean()
	{
	    (*this).clear();
	}

	string ThingSpeak::getString()
	{
	    string ret;
	    for (auto i:*this)
	    {
	        ret+="field"+to_string(i.first)+"="+i.second+"&";
	    }
	    if (!ret.empty())
            ret.pop_back(); // rimuove ultimo '&'
	    return ret;
	    
	}
	
int ThingSpeak::publish(const string &key)
{
	string payload = "https://api.thingspeak.com/update?api_key="+ key+"&"+getString();
//	ESP_LOGI(TAG,"URL:%s",payload.c_str());
	string resp;
	int ret = getSynch(payload, resp, 443);
//	ESP_LOGI(TAG,"RESP:%d",ret);
	//lastRes="["+to_string(ret)+"] "+ getHeaders()["Date"];
	return ret;
}

std::string ThingSpeak::uriEscape(const std::string& input)
{
    std::ostringstream escaped;
    escaped << std::hex << std::uppercase;

    for (unsigned char c : input)
    {
        // caratteri non da codificare (RFC 3986)
        if (std::isalnum(c) ||
            c == '-' || c == '_' || c == '.' || c == '~')
        {
            escaped << c;
        }
        else
        {
            escaped << '%' << std::setw(2) << std::setfill('0')
                     << (int)c;
        }
    }

    return escaped.str();
}