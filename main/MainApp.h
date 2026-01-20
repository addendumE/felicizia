#pragma once

#include "WifiManager.h"
#include "Websocket.h"
#include "ThingSpeak.h"
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
class MyPersistence: public Persistance {
public:
	MyPersistence(Protocol &prot, ObjManager &om, Nvs &nvs):protocol(prot),om(om),nvs(nvs){};
	virtual ~MyPersistence(){};
	void changeNotify(string objId, PropertyId p)
	{
		protocol.propChangeNotification(objId,p);
		om.propChangeNotification(objId,p);
	}
	bool loadFloat(string id, PropertyId p, float &value)
	{
		id += "_"+to_string(p);
		string s;
		bool ret = nvs.getString(id,s);
		value = atof(s.c_str());
		return ret;
	}
	bool loadInt(string id, PropertyId p, int &value)
	{
		id += "_"+to_string(p);
		string s;
		bool ret = nvs.getString(id,s);
		value = atoi(s.c_str());
		return ret;
	}
	bool loadBool(string id, PropertyId p, bool &value)
	{
		id += "_"+to_string(p);
		string s;
		bool ret = nvs.getString(id,s);
		if (ret)
			value =  (s=="1");
		else
			value = false;
		return ret;
	}
	bool loadString(string id, PropertyId p, string &value)
	{
		id += "_"+to_string(p);
		return nvs.getString(id,value);
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
};


class PublishData: public Channel
{
public:
	PublishData()
	{
		push_back("1.0");
		push_back("2.0");
		push_back("3.0");
	}
	virtual ~PublishData(){};
};


class MainApp: public WifiManager,  public ThingSpeak {
public:
	MainApp();
	virtual ~MainApp();
	void start();
private:
	string version;
	Timer reboot;
	Timer publishTimer;
	Nvs nvs;
	ObjManager *objManager;
	Protocol *protocol;
	MyPersistence *persistance;
	DataManager *dataManager;
	Hal hal;
	void publishData();
	void onMode(WifiManager::Mode mode);
};
