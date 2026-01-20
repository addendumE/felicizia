#include "DataManager.h"
#include <math.h>
static const char *TAG="DM";

DataManager::DataManager (ObjManager &om,Persistance &persistance,Hal &hal):
Thread("dataManager"),
om(om),
persistance(persistance),
hal(hal),
adc(0,0),
storageLevelMeter(1,4),
/*trenchLevelMeter(2,5),*/
device( new Device("dev","device",persistance)),
vbatt(new AnalogInputCal("vbatt","battery voltage",persistance,UNIT_VOLTAGE,1)),
ppm(new AnalogInputCal("ppm","storage condictuvuty",persistance,UNIT_CONDUCTIVITY,1)),
trenchLevel(new AnalogInputCal("trenchLevel","trench level",persistance,UNIT_DISTANCE,1)),
storageLevel(new AnalogInputCal("storageLevel","storage level",persistance,UNIT_DISTANCE,1)),
storageTemperature(new AnalogInputCal("storageTemperature","storage temperature",persistance,UNIT_TEMPERATURE,1)),
airTemperature(new AnalogInputCal("airTemperature","air temperature",persistance,UNIT_TEMPERATURE,1)),
waterTemperature(new AnalogInputCal("waterTemperature","water temperature",persistance,UNIT_TEMPERATURE,1)),
pumpOnTime(new AnalogInput("pumpOnTime","pump on time",persistance,UNIT_MINUTES,1)),
pumpDailyCycles(new AnalogInput("pumpDailyCycles","pump daily cycles",persistance,UNIT_NONE,0)),
trenchPumpTh("trenchPumpTh","trench pump",persistance,UNIT_TEMPERATURE,1,false,Threshold::MODE_OVER),
storagePumpTh("storagePumpTh","storage pump",persistance,UNIT_DISTANCE,1,false,Threshold::MODE_OVER),
storageFullTh("storageFullTh","storage full",persistance,UNIT_DISTANCE,1,false,Threshold::MODE_OVER),
dayTimeTh("dayTimeTh","day time",persistance,UNIT_HOURS,1,false,Threshold::MODE_IN),
trenchPumpCmd("trenchPumpCmd","trench pump",persistance,0,false,hal),
storagePumpCmd("storagePumpCmd","storage pump",persistance,0,false,hal)
{
    ow.init(6);
    om.addObject(device);
    //om.addObject(vbatt);
    //om.addObject(ppm);
    //om.addObject(trenchLevel);
    om.addObject(storageLevel);
    om.addObject(storageTemperature);
    om.addObject(airTemperature);
    om.addObject(waterTemperature);
    om.addObject(pumpOnTime);
    om.addObject(pumpDailyCycles);
    om.addObject(&trenchPumpTh);
    om.addObject(&storagePumpTh);
    om.addObject(&storageFullTh);
    om.addObject(&trenchPumpCmd);
    om.addObject(&storagePumpCmd);
    om.addObject(&dayTimeTh);
    
    start();
}

DataManager::~DataManager()
{

}

void DataManager::loop()
{
    while (true)
    {
        float tmpMeas;
        bool tmpResult;
        msleep(1000);

        // DEVICE
        device->setFreeHeap(hal.heapOccupation());

        // STORAGE LEVEL
        storageLevel->setValue(storageLevelMeter.getMeasure(),storageLevelMeter.getMeasure()>=5.9f);

        // TRENCH LEVEL
        // trenchLevel->setValue(trenchLevelMeter.getMeasure(),trenchLevelMeter.getMeasure()>=5.9f);

        // 
        // AIR TEMPERATURE
        tmpResult = ow.readTemperature(0,tmpMeas);
        airTemperature->setValue(tmpMeas,!tmpResult);

        //WATER TEMERATURE
        tmpResult = ow.readTemperature(1,tmpMeas);
        waterTemperature->setValue(tmpMeas,!tmpResult);

        //trench pump threshold
        trenchPumpTh.setValue(trenchLevel->getValue());

        //storage pump threshold
        storagePumpTh.setValue(storageLevel->getValue());

        //storage full threshold
        storageFullTh.setValue(storageLevel->getValue());


        pumpOnTime->setValue(5);
        pumpDailyCycles->setValue(5);
        time_t now = time(NULL);         // Get current time
        struct tm *t = localtime(&now);  // Convert to local time structure

              
    

        dayTimeTh.setValue((float)t->tm_hour+(float)t->tm_min/60.0);
        
        trenchPumpCmd.setValue(trenchPumpTh.getValue() && !storageFullTh.getValue());

        
        float hStart,hEnd;
        dayTimeTh.getThValue(hStart,hEnd);
        int minStart = round(hStart*60.0);
        int minEnd = round(hEnd*60.0);
        int minutes = t->tm_hour*24 + t->tm_min;
        if (minutes >=minStart &&  minutes < minEnd)
        {
            storagePumpCmd.setValue(
                dayTimeTh.getValue() && /* in interval */
                storagePumpTh.getValue() && /* storage level ok*/
                ((minutes-minStart) % ((minEnd-minStart)/(int)pumpDailyCycles->getValue())) < (int)pumpOnTime->getValue()); /* pulse */
        }
        else
        {
            storagePumpCmd.setValue(false);
        }
    }
}