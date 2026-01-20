/*
 * Adc.cpp
 *
 *  Created on: 29 ott 2024
 *      Author: maurizio
 */

#include "Adc.h"
static const char *TAG = "Adc";
#include "driver/gpio.h"

Adc::Adc() {
	init_config = {};
	init_config.unit_id = ADC_UNIT_1;
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc1_handle));
}

Adc::~Adc() {
	for (auto &i:calibration_handlers)
	{
#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    ESP_LOGI(TAG, "deregister %s calibration scheme", "Curve Fitting");
    ESP_ERROR_CHECK(adc_cali_delete_scheme_curve_fitting(i.second));

#elif ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    ESP_LOGI(TAG, "deregister %s calibration scheme", "Line Fitting");
    ESP_ERROR_CHECK(adc_cali_delete_scheme_line_fitting(i.second));
#endif
	}
}

bool Adc::init(int ch, int atten)
{
	bool ret = true;
	take();
	attenuations[ch] = (adc_atten_t)atten;
	adc_cali_handle_t cal;
	gpio_set_direction((gpio_num_t)ch, GPIO_MODE_INPUT);         // ADC
    if (calibration_init(ADC_UNIT_1, (adc_channel_t)ch, (adc_atten_t)atten, &cal))
    {
    	calibration_handlers[ch]=cal;
		adc_oneshot_chan_cfg_t config = {};
		config.bitwidth = ADC_BITWIDTH_DEFAULT;
		config.atten = (adc_atten_t)atten;
		ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, (adc_channel_t)ch, &config));
    	ESP_LOGI(TAG, "ADC Channel[%d] calibrated",ch);
    	ret = true;
    }
    else
    {
    	ESP_LOGE(TAG, "ADC Channel[%d] calibration failed witg gain %d",ch,atten);
    }
    give();
    return ret;
}


float Adc::readVoltage(int ch)
{
	int voltage;
	int raw=0;
	take();
	if (attenuations.count(ch) > 0)
	{
		ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, (adc_channel_t)ch, &raw));
		if (calibration_handlers.count(ch) > 0)
		{
			ESP_ERROR_CHECK(adc_cali_raw_to_voltage(calibration_handlers.at(ch), raw, &voltage));
			//ESP_LOGI(TAG, "ADC Channel[%d] Cali Voltage: [%d] %d mV", ch, raw,voltage);
		}
		else
		{
			ESP_LOGE(TAG, "ADC Channel[%d] not calibrated", ch);
		}
	}
	else
	{
		ESP_LOGW(TAG, "ADC Channel[%d] not initialised", ch);
	}
	give();
	return voltage/1000.0;

}


bool Adc::calibration_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t *out_handle)
{
    adc_cali_handle_t handle = NULL;
    esp_err_t ret = ESP_FAIL;
    bool calibrated = false;

#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    if (!calibrated) {
        ESP_LOGI(TAG, "calibration scheme version is %s", "Curve Fitting");
        adc_cali_curve_fitting_config_t cali_config = {
            .unit_id = unit,
            .chan = channel,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_curve_fitting(&cali_config, &handle);
        if (ret == ESP_OK) {
            calibrated = true;
        }
    }
#endif

#if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    if (!calibrated) {
        ESP_LOGI(TAG, "calibration scheme version is %s", "Line Fitting");
        adc_cali_line_fitting_config_t cali_config = {
            .unit_id = unit,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_line_fitting(&cali_config, &handle);
        if (ret == ESP_OK) {
            calibrated = true;
        }
    }
#endif

    *out_handle = handle;
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Calibration Success");
    } else if (ret == ESP_ERR_NOT_SUPPORTED || !calibrated) {
        ESP_LOGW(TAG, "eFuse not burnt, skip software calibration");
    } else {
        ESP_LOGE(TAG, "Invalid arg or no memory");
    }

    return calibrated;
}



