#include <OneWire.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_check.h"
static const char *TAG="OneW";

OneWire::OneWire():
		Thread("oneWire"),
		temperatures({}),
		good({}),
		ds18b20_device_num(0),
		bus(NULL),
		bus_config({}),
		rmt_config({})
{
	for (int i=0; i<EXAMPLE_ONEWIRE_MAX_DS18B20; i++)
	{
		temperatures[i] = 0.0f;
	}
};

OneWire::~OneWire()
{
};


bool OneWire::init(int gpio)
{
	 bus_config.bus_gpio_num = gpio;
	 rmt_config.max_rx_bytes = 10; // 1byte ROM command + 8byte ROM number + 1byte device command
	 ESP_ERROR_CHECK(onewire_new_bus_rmt(&bus_config, &rmt_config, &bus));
	gpio_pullup_en((gpio_num_t) gpio);
	 gpio_set_drive_capability((gpio_num_t) gpio, GPIO_DRIVE_CAP_3);
	 ESP_LOGI(TAG, "1-Wire bus installed on GPIO%d", gpio);
	 onewire_device_iter_handle_t iter = NULL;
	 onewire_device_t next_onewire_device;
	 esp_err_t search_result = ESP_OK;
	 // create 1-wire device iterator, which is used for device search
	 ESP_ERROR_CHECK(onewire_new_device_iter(bus, &iter));
	 ESP_LOGI(TAG, "Device iterator created, start searching...");
	 do {
		 search_result = onewire_device_iter_get_next(iter, &next_onewire_device);
		 if (search_result == ESP_OK) { // found a new device, let's check if we can upgrade it to a DS18B20
			 ds18b20_config_t ds_cfg = {};
             onewire_device_address_t address;
			 if (ds18b20_new_device_from_enumeration(&next_onewire_device, &ds_cfg, &ds18b20s[ds18b20_device_num]) == ESP_OK) {
                ds18b20_get_device_address(ds18b20s[ds18b20_device_num], &address);
                ESP_LOGI(TAG, "Found a DS18B20[%d], address: %016llX", ds18b20_device_num, address);
                ds18b20_device_num++;
            } else {
                ESP_LOGI(TAG, "Found an unknown device, address: %016llX", next_onewire_device.address);
            }
		 }
	} while (search_result != ESP_ERR_NOT_FOUND);
	ESP_ERROR_CHECK(onewire_del_device_iter(iter));

    ESP_LOGI(TAG, "Searching done, %d DS18B20 device(s) found", ds18b20_device_num);
    // set resolution for all DS18B20s
      for (int i = 0; i < ds18b20_device_num; i++) {
         // set resolution
          ESP_ERROR_CHECK(ds18b20_set_resolution(ds18b20s[i], DS18B20_RESOLUTION_11B));
      }

      Thread::start();
      return true;
}

void OneWire::loop(void)
{
	while(ds18b20_device_num > 0)
	{
		esp_err_t ret = ds18b20_trigger_temperature_conversion_for_all(bus);
		Lock::take();
		for (int i=0; i<ds18b20_device_num; i++)
		{
			good[i]=false;
			if (ret == ESP_OK)
			{
				float res = -100.0f;
				ret = ds18b20_get_temperature(ds18b20s[i], &res);
				if (ret == ESP_OK && res >= -20.0f)
				{
					ESP_LOGI(TAG, "temperature read from DS18B20[%d]: %f °C", i, res);
					temperatures[i] = res;
					good[i]=true;
				}
				else
				{
					ESP_LOGE(TAG, "ds18b20_get_temperature %d",ret);
				}
			}
			else
			{
				ESP_LOGE(TAG, "ds18b20_trigger_temperature_conversion %d",ret);
			}
		}
		Lock::give();
		vTaskDelay(pdMS_TO_TICKS(500));

	}
}
