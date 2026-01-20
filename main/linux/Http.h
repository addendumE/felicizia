#pragma once

#include <stdint.h>
#include <vector>
#include <string>
#include <map>

class Http {
public:
	Http();
	virtual ~Http();
	int get(const std::string url, uint64_t &, int port = 0);
	int getSynch(const std::string url, std::string &resp, int port = 0);
	virtual bool onData(const std::string &resp) { return false;}
	std::map <std::string,std::string> &getHeaders(){return headers;}

private:
	std::map <std::string,std::string> headers;

    static size_t curl_write_data(char *ptr, size_t size, size_t nmemb, void *userdata);
	std::string data;

};
