#include "Thread.h"
#include "ObjManager.h"
#include "Persistance.h"
#include "AnalogInputCal.h"
#include "DigitalOutput.h"
#include "Device.h"
#include "Threshold.h"
#include "ADS115.h"
#include "OneWire.h"
#include "UsRange.h"
#include "Hal.h"
#pragma once

class DataManager: public Thread
{
public:
    DataManager (ObjManager &om, Persistance &persistance, Hal &hal);
    virtual ~DataManager();
private:
    void loop();
    ObjManager &om;
    Persistance &persistance;
    Hal &hal;
    Ads1115 adc;
   	UsRange storageLevelMeter;
   /*	UsRange trenchLevelMeter;*/
    OneWire ow;
    Device *device;
    AnalogInputCal *vbatt;
    AnalogInputCal *ppm;
    AnalogInputCal *trenchLevel;
    AnalogInputCal *storageLevel;
    AnalogInputCal *storageTemperature;
    AnalogInputCal *airTemperature;
    AnalogInputCal *waterTemperature;
    AnalogInput *pumpOnTime;
    AnalogInput *pumpDailyCycles;
    Threshold trenchPumpTh;
    Threshold storagePumpTh;
    Threshold storageFullTh;
    Threshold dayTimeTh;
    DigitalOutput trenchPumpCmd;
    DigitalOutput storagePumpCmd;
};
