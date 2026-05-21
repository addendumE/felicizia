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

class MyPersistence: public Persistance {
public:
	MyPersistence(Protocol &protocol, ObjManager &om, Nvs &nvs, ThingSpeak &ts, MqttClient &mqtt, Websocket &websocket):
		protocol(protocol),
		om(om),
		nvs(nvs),
		ts(ts),
		mqtt(mqtt),
		websocket(websocket)
	{

	};
	virtual ~MyPersistence(){};
	void setUid(std::string _uid)
	{
		pubtopic = _uid+"/Uplink";
	}
	void changeNotify(string objId, PropertyId p)
	{
		string msg =protocol.propChangeNotification(objId,p);
		mqtt.publish(pubtopic,msg);
		websocket.send(msg);
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
	Nvs nvs;
	ThingSpeak ts;
	ObjManager *objManager;
	MqttClient *mqtt;
	MyProtocol *protocol;
	MyPersistence *persistance;
	
	Hal hal;
	DataManager *dataManager;
	void onMode(WifiManager::Mode mode);
	void startMqtt();
	void onMessage(const string&);
	void onOTAenter(){};
	void onOTAexit(){
		vTaskDelay(pdMS_TO_TICKS(1000));
		esp_restart();
	};
	void onConfigRead(string &s)
	{
		objManager->getConf(s);
	}
	bool onConfigWrite(string &s) {
		return objManager->setConf(s);
	}

};
