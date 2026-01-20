/*
 * Adc.cpp
 *
 *  Created on: 29 ott 2024
 *      Author: maurizio
 */

#include "Hal.h"
#include "multi_heap.h"
#include "esp_heap_caps.h"
#include "driver/gpio.h"
static const char *TAG = "Hal";


Hal::Hal() {
	
}

Hal::~Hal() {
	
}

float Hal::heapOccupation()
{
    multi_heap_info_t info;
    heap_caps_get_info(&info, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT); // internal RAM, memory capable to store data or to create new task
    return (100.0f*info.total_allocated_bytes)/(info.total_allocated_bytes+info.total_free_bytes);
}


bool Hal::requestOutput(int idx, bool val)
{
    ESP_ERROR_CHECK(gpio_set_direction((gpio_num_t)idx  , GPIO_MODE_OUTPUT));
   	ESP_ERROR_CHECK(gpio_set_level((gpio_num_t)idx, val));
    return true;
}

bool Hal::setOutput(int idx, bool val)
{
   	ESP_ERROR_CHECK(gpio_set_level((gpio_num_t)idx, val));
    return true;
}
