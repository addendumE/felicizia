#include <OneWire.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_check.h"
static const char *TAG="OneW";

OneWire::OneWire():
		Thread("oneWire",2560),
		temperatures({0.0f}),
		errCnt({0}),
		ds18b20_device_num(0),
		bus(NULL),
		bus_config({}),
		rmt_config({})
{
};

OneWire::~OneWire()
{
};

bool OneWire::searchDevices(std::vector<onewire_device_address_t>& devices)
{
    devices.clear();

    onewire_device_iter_handle_t iter = nullptr;

    esp_err_t ret = onewire_new_device_iter(bus, &iter);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Cannot create 1-Wire iterator: %s",
                 esp_err_to_name(ret));
        return false;
    }

    while (true) {
        onewire_device_t device;

        ret = onewire_device_iter_get_next(iter, &device);

        if (ret == ESP_ERR_NOT_FOUND) {
            break;
        }

        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "1-Wire search error: %s",
                     esp_err_to_name(ret));
            onewire_del_device_iter(iter);
            return false;
        }

        // Evita duplicati, utile anche nel caso di una
        // futura implementazione con più scansioni.
        bool duplicate = false;

        for (const auto& address : devices) {
            if (address == device.address) {
                duplicate = true;
                break;
            }
        }

        if (!duplicate) {
            devices.push_back(device.address);

            ESP_LOGI(TAG,
                     "Found device: %016llX",
                     device.address);
        }
    }

    onewire_del_device_iter(iter);

    return true;
}

bool OneWire::init(int gpio)
{
	 bus_config.bus_gpio_num = gpio;
     bus_config.flags.en_pull_up =1;
	 rmt_config.max_rx_bytes = 10; // 1byte ROM command + 8byte ROM number + 1byte device command
	 ESP_ERROR_CHECK(onewire_new_bus_rmt(&bus_config, &rmt_config, &bus));
	 ESP_LOGI(TAG, "1-Wire bus installed on GPIO%d", gpio);
	 
	 std::vector<onewire_device_address_t> addresses;
	 for (auto i=0;i<5;i++)
	 {
		searchDevices(addresses);
		if (addresses.size() == EXAMPLE_ONEWIRE_MAX_DS18B20)
		{
			break;
		}
		else
		{
			vTaskDelay(pdMS_TO_TICKS(50));
		}
	}

	ds18b20_device_num = 0;

	for (const auto& address : addresses) {

		onewire_device_t device = {
			.address = address,
		};

		ds18b20_config_t ds_cfg = {};

		if (ds18b20_new_device_from_enumeration(
				&device,
				&ds_cfg,
				&ds18b20s[ds18b20_device_num]) == ESP_OK) {

			ESP_LOGI(TAG,
					"Found DS18B20[%d]: %016llX",
					ds18b20_device_num,
					address);
          ESP_ERROR_CHECK(ds18b20_set_resolution(ds18b20s[ds18b20_device_num], DS18B20_RESOLUTION_11B));
			ds18b20_device_num++;
		} else {
			ESP_LOGI(TAG,
					"Unknown 1-Wire device: %016llX",
					address);
		}
	}

    ESP_LOGI(TAG, "Searching done, %d DS18B20 device(s) found", ds18b20_device_num);
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
			
			if (ret == ESP_OK)
			{
				float res = -100.0f;
				ret = ds18b20_get_temperature(ds18b20s[i], &res);
				if (ret == ESP_OK && res >= -20.0f)
				{
					ESP_LOGI(TAG, "temperature read from DS18B20[%d]: %f °C", i, res);
					temperatures[i] = res;
					errCnt[i]=0;
				}
				else
				{
					errCnt[i]++;
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
