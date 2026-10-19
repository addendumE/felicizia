#pragma once
#include <stdint.h>

typedef int esp_err_t;
#define ESP_OK 0
#define ESP_FAIL -1
#define ESP_ERR_OTA_VALIDATE_FAILED -2

inline const char* esp_err_to_name(esp_err_t code) {
    return (code == ESP_OK) ? "ESP_OK" : "ESP_FAIL";
}

typedef enum {
    ESP_RST_UNKNOWN = 0,
    ESP_RST_POWERON,
    ESP_RST_SW,
} esp_reset_reason_t;

inline esp_reset_reason_t esp_reset_reason(void) {
    return ESP_RST_POWERON;
}
