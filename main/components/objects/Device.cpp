/*
 * Device.cpp
 *
 *  Created on: Nov 24, 2025
 *      Author: maurizio
 */

#include "Device.h"
#include "esp_log.h"
#include "string.h"
Device::Device(string id,string name, Persistance &p):
	Base(TYPE_DEVICE,id,name,p)
{
	getUid();
	addStringProperty(PROP_VALUE,Property::MODE_READONLY,"OK");
	addFloatProperty(PROP_FREE_HEAP,Property::MODE_READONLY, 0,UNIT_PERCENT,1);
	addStringProperty(PROP_SSID,Property::MODE_WRITABLE_PERSISTENT,"");
	addStringProperty(PROP_SSID_KEY,Property::MODE_WRITABLE_PERSISTENT,"");
	addStringProperty(PROP_TS_KEY,Property::MODE_WRITABLE_PERSISTENT,"");
	addStringProperty(PROP_MQTT_URI,Property::MODE_WRITABLE_PERSISTENT,"mqtt://broker.hivemq.com:1883");
	addStringProperty(PROP_SW_VERSION,Property::MODE_READONLY,BUILD_VERSION);
	addStringProperty(PROP_UID,Property::MODE_READONLY,uid);
}

Device::~Device() {
}

void Device::setFreeHeap(float fh)
{
	setPropertyValueFloat(PROP_FREE_HEAP,fh);
}

#ifndef LINUX_PLATFORM
#include "mbedtls/md5.h"
#include "esp_mac.h"

void Device::getUid()
{
    uint8_t mac[6];
    esp_efuse_mac_get_default(mac);
    const char *salt = "FELICIZIA";
    uint8_t buffer[64];
    size_t len = 0;
    memcpy(buffer + len, salt, strlen(salt));
    len += strlen(salt);
    memcpy(buffer + len, mac, sizeof(mac));
    len += sizeof(mac);
    // MD5
    uint8_t digest[16];
    mbedtls_md5(buffer, len, digest);
    // primi 8 byte => 16 char hex
	uid.clear();
	for (int i = 0; i < 8; i++)
	{
		char tmp [3];
		snprintf(tmp, sizeof(tmp),"%02X",digest[i]);
		uid.append(tmp);
	}
}
#else
void Device::getUid()
{
	uid="LINUX_PLATDORM_DEBUG";
}
#endif