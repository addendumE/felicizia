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


void nvs_example(void)
{
    esp_err_t ret;
    nvs_handle_t handle;
    const char* NAMESPACE = "storage";
    const char* KEY       = "my_key";

    // --- APRI ---
    ret = nvs_open(NAMESPACE, NVS_READWRITE, &handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Errore apertura: %s", esp_err_to_name(ret));
        return;
    }
    ESP_LOGI(TAG, "NVS aperto");

    // --- SCRIVI ---
    ret = nvs_set_str(handle, KEY, "ciao mondo");
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Errore scrittura: %s", esp_err_to_name(ret));
        nvs_close(handle);
        return;
    }

    // commit obbligatorio dopo la scrittura
    ret = nvs_commit(handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Errore commit: %s", esp_err_to_name(ret));
        nvs_close(handle);
        return;
    }
    ESP_LOGI(TAG, "Scrittura OK");

    // --- LEGGI ---
    size_t size = 0;
    ret = nvs_get_str(handle, KEY, NULL, &size);  // prima passata: ottieni dimensione
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Errore lettura dimensione: %s", esp_err_to_name(ret));
        nvs_close(handle);
        return;
    }

    char* value = malloc(size);
    ret = nvs_get_str(handle, KEY, value, &size);  // seconda passata: leggi valore
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Letto: %s", value);
    } else {
        ESP_LOGE(TAG, "Errore lettura: %s", esp_err_to_name(ret));
    }
    free(value);

    // --- CHIUDI ---
    nvs_close(handle);
    ESP_LOGI(TAG, "NVS chiuso");
}
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
    //esp_log_level_set("*", ESP_LOG_ERROR);
    //esp_log_level_set("NVS", ESP_LOG_INFO);
    ESP_ERROR_CHECK(i2cdev_init());
    startup();
}
