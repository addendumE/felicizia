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
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_task_wdt.h"
#include "esp_core_dump.h"
#include <i2cdev.h>
static const char *TAG = "main";

static void check_coredump(void)
{
    esp_err_t err = esp_core_dump_image_check();
    if (err == ESP_OK) {
        size_t addr = 0;
        size_t size = 0;
        if (esp_core_dump_image_get(&addr, &size) == ESP_OK) {
            ESP_LOGW(TAG, "Coredump trovato in flash: offset 0x%zx, dimensione %zu byte", addr, size);
        }
#if CONFIG_ESP_COREDUMP_ENABLE_TO_FLASH && CONFIG_ESP_COREDUMP_DATA_FORMAT_ELF
        char panic_reason[128] = {0};
        if (esp_core_dump_get_panic_reason(panic_reason, sizeof(panic_reason)) == ESP_OK) {
            ESP_LOGW(TAG, "Motivo del crash: %s", panic_reason);
        }
        esp_core_dump_summary_t summary;
        if (esp_core_dump_get_summary(&summary) == ESP_OK) {
            ESP_LOGW(TAG, "Crash avvenuto nel task '%s' (PC: 0x%08" PRIx32 ")",
                     summary.exc_task, summary.exc_pc);
        }
#endif
    } else if (err == ESP_ERR_NOT_FOUND) {
        ESP_LOGI(TAG, "Nessun coredump presente in flash.");
    } else {
        ESP_LOGE(TAG, "Coredump presente ma non integro: %s", esp_err_to_name(err));
    }
}

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
    esp_log_level_set("main", ESP_LOG_INFO);

    check_coredump();

    ESP_ERROR_CHECK(i2cdev_init());
    setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1);
    tzset();
    startup();
}
