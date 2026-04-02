#include "Thread.h"
#include "ObjManager.h"
#include "Persistance.h"
#include "AnalogInput.h"
#include "DigitalOutput.h"
#include "Device.h"
#include "Threshold.h"
#ifndef LINUX_PLATFORM
#include "ads111x.h"
#include "mcp23x17.h"
#else
typedef enum
{
    ADS111X_MUX_0_1 = 0, //!< positive = AIN0, negative = AIN1 (default)
    ADS111X_MUX_0_3,     //!< positive = AIN0, negative = AIN3
    ADS111X_MUX_1_3,     //!< positive = AIN1, negative = AIN3
    ADS111X_MUX_2_3,     //!< positive = AIN2, negative = AIN3
    ADS111X_MUX_0_GND,   //!< positive = AIN0, negative = GND
    ADS111X_MUX_1_GND,   //!< positive = AIN1, negative = GND
    ADS111X_MUX_2_GND,   //!< positive = AIN2, negative = GND
    ADS111X_MUX_3_GND,   //!< positive = AIN3, negative = GND
} ads111x_mux_t;
#endif
#include "OneWire.h"
#include "UsRange.h"
#include "Hal.h"
#pragma once

class DataManager: public Thread
{
public:
    DataManager (ObjManager &om, Persistance &persistance, Hal &hal);
    virtual ~DataManager();
    string getSSID();
    string getPwd();
    string getKey();
private:
    void loop();
    ObjManager &om;
    OneWire ow;
#ifndef LINUX_PLATFORM    
    i2c_dev_t mcp23x17Dev;
    i2c_dev_t ads11xDev;
#endif
    Persistance &persistance;
    Hal &hal;
   	UsRange storageLevelMeter;
   	UsRange trenchLevelMeter;
    Device device;
    AnalogInput vbatt;
    AnalogInput ppm;
    AnalogInput trenchLevel;
    AnalogInput storageLevel;
    AnalogInput airTemperature;
    AnalogInput waterTemperature;
    AnalogInput pumpOnTime;
    AnalogInput pumpDailyCycles;
    Threshold svuotamentoCanaleTh;
    Threshold riempimentoSerbatoioTh;
    Threshold svuotamentoSerbatoioTh;
    Threshold batteryGoodTh;
    Threshold airTemperatureGoodTh;
    Threshold ppmGoodTh;
    DigitalOutput trenchPumpCmd;
    DigitalOutput storagePumpCmd;
    DigitalOutput ledAlive;
    DigitalOutput ledLowBatt;
    DigitalOutput ledLowFosso;
    DigitalOutput ledTorbidita;
    DigitalOutput ledGelo;
    DigitalOutput ledSerbatoioVuoto;
    DigitalOutput ledErrorePompaFosso;
    DigitalOutput ledErrorePompaSerbatoio;
    bool adcRead(ads111x_mux_t mux, float &res);
    void doInput();
    void doOutput();
    bool pompa_on(int irrigazioni_al_giorno, int durata_secondi);
};
