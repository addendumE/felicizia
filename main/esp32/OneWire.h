#pragma once
#include "onewire_bus.h"
#include "driver/gpio.h"
#include "Thread.h"
#include "Lock.h"
#include "ds18b20.h"
#include <vector>

#define EXAMPLE_ONEWIRE_MAX_DS18B20 2

class OneWire: public Thread, public Lock {
public:
	OneWire();
	virtual ~OneWire();
	bool init(int);
	bool readTemperature(int i, float &res) {
		bool ret;
		Lock::take();
		res = temperatures[i];
		ret = errCnt[i]>10;
		Lock::give();
		return ret;
	}
	bool readUID(int i, std::string& uid)
	{
		uid ="";
    	if (i < 0 || i >= ds18b20_device_num)
        	return false;

    	if (ds18b20s[i] == nullptr)
        	return false;

    	onewire_device_address_t address;

    	esp_err_t ret = ds18b20_get_device_address(
        	ds18b20s[i],
        	&address
    	);

    	if (ret != ESP_OK)
        	return false;

	    char buffer[17];

    	snprintf(
        	buffer,
        	sizeof(buffer),
        	"%016llX",
        	static_cast<unsigned long long>(address)
    	);

    	uid = buffer;

	    return true;
	}
private:
	void loop(void);
	float temperatures[EXAMPLE_ONEWIRE_MAX_DS18B20];
	int errCnt[EXAMPLE_ONEWIRE_MAX_DS18B20];
	bool searchDevices(std::vector<onewire_device_address_t>& devices);

    int ds18b20_device_num;
    onewire_bus_handle_t bus;
    onewire_bus_config_t bus_config;
    onewire_bus_rmt_config_t rmt_config;
    ds18b20_device_handle_t ds18b20s[EXAMPLE_ONEWIRE_MAX_DS18B20];
};
