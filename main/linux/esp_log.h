#pragma once

#include <stdarg.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#define pdMS_TO_TICKS(x) (x/1000)

#define vTaskDelay(x)
#define esp_restart()
#define gpio_get_level(x) false

#define GPIO_NUM_21 21
#define GPIO_NUM_20 20
#define GPIO_NUM_9 9
#define BUILD_VERSION "VER_LINUX"

typedef enum {
    ESP_LOG_NONE,       /*!< No log output */
    ESP_LOG_ERROR,      /*!< Critical errors, software module can not recover on its own */
    ESP_LOG_WARN,       /*!< Error conditions from which recovery measures have been taken */
    ESP_LOG_INFO,       /*!< Information messages which describe normal flow of events */
    ESP_LOG_DEBUG,      /*!< Extra information which is not necessary for normal use (values, pointers, sizes, etc). */
    ESP_LOG_VERBOSE     /*!< Bigger chunks of debugging information, or frequent messages which can potentially flood the output. */
} esp_log_level_t;

extern void ESP_LOG_LEVEL(esp_log_level_t level, const char *tag, const char *format, ...);

void ESP_LOG_BUFFER_HEX_LEVEL(const char *tag, void *data,size_t size,int level);



#define ESP_LOG_LEVEL_LOCAL(level, tag, format, ...) do {               \
        ESP_LOG_LEVEL(level, tag, format, ##__VA_ARGS__); \
    } while(0)


#define ESP_LOGE( tag, format, ... ) ESP_LOG_LEVEL_LOCAL(ESP_LOG_ERROR,   tag, format, ##__VA_ARGS__)
#define ESP_LOGW( tag, format, ... ) ESP_LOG_LEVEL_LOCAL(ESP_LOG_WARN,    tag, format, ##__VA_ARGS__)
#define ESP_LOGI( tag, format, ... ) ESP_LOG_LEVEL_LOCAL(ESP_LOG_INFO,    tag, format, ##__VA_ARGS__)
#define ESP_LOGD( tag, format, ... ) ESP_LOG_LEVEL_LOCAL(ESP_LOG_DEBUG,   tag, format, ##__VA_ARGS__)
#define ESP_LOGV( tag, format, ... ) ESP_LOG_LEVEL_LOCAL(ESP_LOG_VERBOSE, tag, format, ##__VA_ARGS__)
