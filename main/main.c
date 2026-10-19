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
#include <time.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_task_wdt.h"
#include <i2cdev.h>
static const char *TAG = "main";

extern void startup();
void app_main(void)
{
    esp_task_wdt_config_t twdt_config = {
        .timeout_ms = 30000, // 10000 ms = 10 secondi
        .idle_core_mask = (1 << 0),
        .trigger_panic = true
    };
    
    // Inizializza o riconfigura il watchdog
    esp_task_wdt_reconfigure(&twdt_config);
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
	    ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);
    esp_log_level_set("*", ESP_LOG_ERROR);
    //esp_log_level_set("OneW", ESP_LOG_INFO);
    //esp_log_level_set("Wifi", ESP_LOG_INFO);
   // esp_log_level_set("MqttClient", ESP_LOG_INFO);
    esp_log_level_set("APP", ESP_LOG_INFO);
    esp_log_level_set("OneW", ESP_LOG_INFO);
   // esp_log_level_set("DM", ESP_LOG_INFO);
    ESP_ERROR_CHECK(i2cdev_init());
    setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1);
    tzset();
    startup();
}
