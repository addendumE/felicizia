#pragma once
#include <Lock.h>
#include "soc/soc_caps.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include <map>
#include <Lock.h>

using namespace std;

class Adc : public Lock{
public:
	Adc();
	virtual ~Adc();
	bool init(int ch, int atten);
	float readVoltage(int ch);
private:
	 adc_oneshot_unit_handle_t adc1_handle;
	 adc_oneshot_unit_init_cfg_t init_config;
	 bool calibration_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t *out_handle);
	 map <int,adc_atten_t> attenuations;
	 map <int,adc_cali_handle_t> calibration_handlers;
};
