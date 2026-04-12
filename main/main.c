/*
 * SPDX-FileCopyrightText: 2021-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

 /*

html-minifier-terser index.full.html \
  --collapse-whitespace \
  --remove-comments \
  --remove-optional-tags \
  --remove-redundant-attributes \
  --remove-script-type-attributes \
  --remove-style-link-type-attributes \
  --minify-js true \
  --minify-css true \
  -o ../HTML/index.html
 
 */
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include <i2cdev.h>
static const char *TAG = "main";

extern void startup();

#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
	    ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);
    //nvs_example();
    esp_log_level_set("*", ESP_LOG_ERROR);
    //esp_log_level_set("OneW", ESP_LOG_INFO);
    //esp_log_level_set("Wifi", ESP_LOG_INFO);
   // esp_log_level_set("NVS", ESP_LOG_INFO);
    ESP_ERROR_CHECK(i2cdev_init());
    startup();
}
