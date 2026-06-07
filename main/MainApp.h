#pragma once

#include "WifiManager.h"
#include "Websocket.h"
#include "Protocol.h"
#include "AnalogInput.h"
#include "ObjManager.h"
#include "DataManager.h"
#include "Nvs.h"
#include "Timer.h"
#include "Hal.h"
#include <cJSON.h>
#include "Device.h"
#include "DigitalOutput.h"
#include "UsRange.h"
#include "MqttClient.h"

#include "esp_ota_ops.h"
#include "esp_flash_partitions.h"
#include "esp_partition.h"
#include "esp_image_format.h"


class MyPersistence: public Persistance {
public:
	MyPersistence(Protocol &protocol, ObjManager &om, Nvs &nvs, ThingSpeak &ts, MqttClient &mqtt, Websocket &websocket):
		protocol(protocol),
		om(om),
		nvs(nvs),
		ts(ts),
		mqtt(mqtt),
		websocket(websocket),
		timerPending(false),
		propChangeTimer("TIM",100)
{

		propChangeTimer.onExpired([&]()
		{
			timerPending = false;
			string msg =protocol.commitPropChange();
			if (!msg.empty())
			{
				mqtt.publish(pubtopic,msg);
				//ESP_LOGI("PER","mqtt msg %s %s",pubtopic.c_str(),msg.c_str());
				websocket.send(msg);	
			}
		});
	};
	virtual ~MyPersistence(){};
	void setUid(std::string _uid)
	{
		pubtopic = _uid+"/Uplink";
	}
	void changeNotify(string objId, PropertyId p)
	{
		protocol.propChangeNotification(objId,p);
		if (!timerPending) {
			timerPending = true;
			propChangeTimer.start();
		}
		//string msg =protocol.commitPropChange();
		//mqtt.publish(pubtopic,msg);
		//websocket.send(msg);
		//om.propChangeNotification(objId,p);
	}
	bool loadFloat(string id, PropertyId p, float &value)
	{
		id += "_"+to_string(p);
		string s;
		bool ret = nvs.getString(id,s);
		if (ret) value = (float)atof(s.c_str());
		return ret;
	}
	bool loadInt(string id, PropertyId p, int &value)
	{
		id += "_"+to_string(p);
		string s;
		bool ret = nvs.getString(id,s);
		if (ret) value = atoi(s.c_str());
		return ret;
	}
	bool loadBool(string id, PropertyId p, bool &value)
	{
		id += "_"+to_string(p);
		string s;
		bool ret = nvs.getString(id,s);
		if (ret) value =  (s=="1");
		return ret;
	}
	bool loadString(string id, PropertyId p, string &value)
	{
		id += "_"+to_string(p);
		string s;
		bool ret = nvs.getString(id,s);
		if (ret)
		{
			value = s;
		}
		return ret;
	}
	bool storeFloat(string id, PropertyId p, float value)
	{
		id += "_"+to_string(p);
		char tmp[64];
		snprintf(tmp,sizeof(tmp),"%f",value);
		return nvs.setString(id,string(tmp));
	}
	bool storeInt(string id, PropertyId p, int value)
	{
		id += "_"+to_string(p);
		char tmp[64];
		snprintf(tmp,sizeof(tmp),"%d",value);
		return nvs.setString(id,string(tmp));
	}
	bool storeBool(string id, PropertyId p, bool value)
	{
		id += "_"+to_string(p);
		return nvs.setString(id,(value)? "1":"0");
	}
	bool storeString(string id, PropertyId p, string value)
	{
		id += "_"+to_string(p);
		return nvs.setString(id,value);
	}
private:
	Protocol &protocol;
	ObjManager &om;
	Nvs &nvs;
	ThingSpeak &ts;
	MqttClient &mqtt;
	Websocket &websocket;
	std::string pubtopic;
	bool timerPending;
	Timer propChangeTimer;

};


class MyProtocol: public Protocol {
public:
	MyProtocol(ObjManager &om):Protocol(om),om(om){};
	virtual ~MyProtocol(){};
private:
	ObjManager &om;
	
};

class MainApp: public WifiManager, public Websocket {
public:
	MainApp();
	virtual ~MainApp();
	void start();
private:
	string version;
	Timer reboot;
	Timer otaConfirm;
	Nvs nvs;
	ThingSpeak ts;
	ObjManager *objManager;
	MqttClient *mqtt;
	MyProtocol *protocol;
	MyPersistence *persistance;
	Hal hal;
	DataManager *dataManager;
	esp_ota_handle_t ota_update_handle;
	const esp_partition_t *ota_update_partition;

	void onMode(WifiManager::Mode mode);
	void startMqtt();
	void onMessage(const string&);
	void onOTAenter(){otaStart();};
	void onOTAexit(){
		otaEnd();
		dataManager->setDeviceStatus("REBOOT");
		reboot.start();
	};
	void onOTAdata(const char *data, size_t len)
	{
		otaProcess(data,len);
	}
	void onConfigRead(string &s)
	{
		objManager->getConf(s);
	}
	bool onConfigWrite(string &s) {
		return objManager->setConf(s);
	}

	esp_err_t otaStart();
	esp_err_t otaProcess(const char *data, size_t len);
	esp_err_t otaEnd();
	void check_ota_state();
};
