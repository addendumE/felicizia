#pragma once
#include "onewire_bus.h"
#include "driver/gpio.h"
#include "Thread.h"
#include "Lock.h"
#include "ds18b20.h"
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
		ret = good[i];
		Lock::give();
		return ret;
	}
private:
	void loop(void);
	float temperatures[EXAMPLE_ONEWIRE_MAX_DS18B20];
	bool good[EXAMPLE_ONEWIRE_MAX_DS18B20];

    int ds18b20_device_num;
    onewire_bus_handle_t bus;
    onewire_bus_config_t bus_config;
    onewire_bus_rmt_config_t rmt_config;
    ds18b20_device_handle_t ds18b20s[EXAMPLE_ONEWIRE_MAX_DS18B20];
};
