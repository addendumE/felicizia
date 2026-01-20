#include "esp_log.h"
#include <stdarg.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
    static char buffer[128] = {0};

char *esp_log_system_timestamp(void)
{
    struct timeval tv;
    struct tm timeinfo;

        gettimeofday(&tv, NULL);
        localtime_r(&tv.tv_sec, &timeinfo);

        snprintf(buffer, sizeof(buffer),
                 "%02d:%02d:%02d.%03ld",
                 timeinfo.tm_hour,
                 timeinfo.tm_min,
                 timeinfo.tm_sec,
                 tv.tv_usec / 1000);

        return buffer;
}


void esp_log_writev(esp_log_level_t level,
                   const char *tag,
                   const char *format,
                   va_list args)
{
	char buffer[64];
    struct timeval tv;
    struct tm timeinfo;

    gettimeofday(&tv, NULL);
    localtime_r(&tv.tv_sec, &timeinfo);

            snprintf(buffer, sizeof(buffer),
                     "%02d:%02d:%02d.%03ld",
                     timeinfo.tm_hour,
                     timeinfo.tm_min,
                     timeinfo.tm_sec,
                     tv.tv_usec / 1000);
    printf("%s\t",buffer);
    printf("%s\t",tag);
    switch (level)
    {
    case ESP_LOG_ERROR: printf("E\t"); break;
    case ESP_LOG_WARN: printf("W\t");break;
    case ESP_LOG_INFO:  printf("I\t");break;
    case ESP_LOG_DEBUG: printf("D\t");break;
    case ESP_LOG_VERBOSE:  printf("V\t");break;
    }
    vprintf(format, args);
    putchar('\n');

}


void ESP_LOG_LEVEL(esp_log_level_t level,
                   const char *tag,
                   const char *format, ...)
{
    va_list list;
    va_start(list, format);
    esp_log_writev(level, tag, format, list);
    va_end(list);
}

void ESP_LOG_BUFFER_HEX_LEVEL(const char *tag, void *data,size_t size,int level)
{
	char buffer[64];
	    struct timeval tv;
	    struct tm timeinfo;

	    gettimeofday(&tv, NULL);
	    localtime_r(&tv.tv_sec, &timeinfo);

	            snprintf(buffer, sizeof(buffer),
	                     "%02d:%02d:%02d.%03ld",
	                     timeinfo.tm_hour,
	                     timeinfo.tm_min,
	                     timeinfo.tm_sec,
	                     tv.tv_usec / 1000);
	    printf("%s\t",buffer);
	    printf("%s\t",tag);
	    switch (level)
	    {
	    case ESP_LOG_ERROR: printf("E\t"); break;
	    case ESP_LOG_WARN: printf("W\t");break;
	    case ESP_LOG_INFO:  printf("I\t");break;
	    case ESP_LOG_DEBUG: printf("D\t");break;
	    case ESP_LOG_VERBOSE:  printf("V\t");break;
	    }
	    unsigned char *p = (unsigned char*)data;
	    while(size--)
	    {
	    	printf("%02X ",*p++);
	    }
	    putchar('\n');

}

