#define NO_IIC_DEBUG
#include "Thread.h"
#include "ObjManager.h"
#include "Persistance.h"
#include "AnalogInput.h"
#include "DigitalOutput.h"
#include "DigitalOutputFeedback.h"
#include "DigitalInput.h"
#include "Device.h"
#include "Threshold.h"
#include <functional>
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
#include "ThingSpeak.h"
#pragma once

class DataManager: public Thread
{
public:
    DataManager (ObjManager &om, Persistance &persistance, Hal &hal);
    virtual ~DataManager();
    string getSSID();
    string getPwd();
    string getKey();
    string getMqttUri();
    string getUid();
    void setDeviceStatus(const string &status);
    using ButtonCallBack = std::function<void(void)>;
    void onBootButton(ButtonCallBack cb) {
        bootButtonCallBack = cb;
    }
private:
    ButtonCallBack bootButtonCallBack;

    void loop();
    ObjManager &om;
    OneWire ow;
#ifndef LINUX_PLATFORM    
    i2c_dev_t mcp23x17Dev;
    i2c_dev_t ads11xDev;
#endif
    Persistance &persistance;
    Hal &hal;
   	UsRange livSerbatoioMeter;
   	UsRange livFossoMeter;
    Device device;
    AnalogInput vbatt;
    AnalogInput sccm;
    AnalogInput livFosso;
    AnalogInput livSerbatoio;
    AnalogInput tempAria;
    AnalogInput tempAcqua;
    AnalogInput tempoIrrigazione;
    AnalogInput numeroIrrigazioni;
    Threshold livSvuotamentoCanaleOk;
    Threshold livRiempimentoSerbatoioOk;
    Threshold livIrrigazioneOk;
    Threshold tensioneBatteriaOk;
    Threshold temperaturaAriaOk;
    Threshold conducibilitaOk;
    Threshold orarioOk;
    DigitalOutputFeedback cmdPompaRiempimento;
    DigitalOutputFeedback cmdPompaIrrigazione;
    DigitalOutput ledAlive;
    DigitalOutput ledLowBatt;
    DigitalOutput ledLowFosso;
    DigitalOutput ledTorbidita;
    DigitalOutput ledLowTaria;
    DigitalOutput ledLowSerbatoio;
    DigitalOutput ledErrorePompaFosso;
    DigitalOutput ledErrorePompaSerbatoio;
    DigitalInput feedbackPompaCanale;
    DigitalInput feedbackPompaSerbatoio;
    int last_state;
    size_t loopCnt;
    ThingSpeak ts;
    struct s_old_out
    {
        bool  ledLowBatt:1;
        bool  ledLowFosso:1;
        bool  ledTorbidita:1;
        bool  ledLowTaria:1;
        bool  ledLowSerbatoio:1;
        bool  ledErrorePompaFosso:1;
        bool  ledErrorePompaSerbatoio:1;
        bool  cmdPompaIrrigazione:1;
        bool  cmdPompaRiempimento:1;
        bool  boot:1;
    };
    s_old_out oldOut;
    bool adcRead(ads111x_mux_t mux, float &res);
    void doInput();
    void doOutput();
    bool pompa_on(int irrigazioni_al_giorno, int durata_secondi, float  start, float end);
};
