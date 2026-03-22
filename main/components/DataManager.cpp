#include "DataManager.h"
#include <math.h>
#include <stdint.h>
static const char *TAG="DM";

#define POMPA_SERBATOIO_GPIO    GPIO_NUM_8
#define POMPA_FOSSO_GPIO        GPIO_NUM_9



DataManager::DataManager (ObjManager &om,Persistance &persistance,Hal &hal):
Thread("dataManager"),
om(om),
#ifndef LINUX_PLATFORM
mcp23x17Dev({}),
ads11xDev({}),
#endif
persistance(persistance),
hal(hal),
storageLevelMeter(1,4),
//trenchLevelMeter(1,5),
device("device","device",persistance),
vbatt("vbatt","tensione batteria",persistance,UNIT_VOLTAGE,1),
ppm("ppm","conducibilità",persistance,UNIT_CONDUCTIVITY,1),
trenchLevel("trenchLevel","livello fosso",persistance,UNIT_DISTANCE,1),
storageLevel("storageLevel","livello serbatotio",persistance,UNIT_DISTANCE,1),
airTemperature("airTemperature","temperatura aria",persistance,UNIT_TEMPERATURE,1),
waterTemperature("waterTemperature","temperatura acqua",persistance,UNIT_TEMPERATURE,1),
pumpOnTime("pumpOnTime","tempo pompaggio",persistance,UNIT_MINUTES,1),
pumpDailyCycles("pumpDailyCycles","cicli pompaggio",persistance,UNIT_NONE,0),
svuotamentoCanaleTh("svuotamentoCanaleTh","fosso pronto a svuotamento",persistance,UNIT_DISTANCE,1,false,Threshold::MODE_OVER),
riempimentoSerbatoioTh("riempimentoSerbatoioTh","serbatoio pronto a riempimento",persistance,UNIT_DISTANCE,1,false,Threshold::MODE_OVER),
svuotamentoSerbatoioTh("svuotamentoSerbatoioTh","serbatoio pronto a svuotamento",persistance,UNIT_DISTANCE,1,false,Threshold::MODE_OVER),
batteryGoodTh("batteryGoodTh","batteria ok",persistance,UNIT_VOLTAGE,1,false,Threshold::MODE_OVER),
airTemperatureGoodTh("airTemperatureGoodTh","temperatura aria ok",persistance,UNIT_TEMPERATURE,1,false,Threshold::MODE_OVER),
ppmGoodTh("ppmGoodTh","conducibilità ok",persistance,UNIT_CONDUCTIVITY,1,false,Threshold::MODE_OVER),
trenchPumpCmd("trenchPumpCmd","trench pump",persistance,0,false),
storagePumpCmd("storagePumpCmd","storage pump",persistance,0,false),
ledAlive("ledAlive","led alive",persistance,0,false),
ledLowBatt("ledLowBatt","led low batt",persistance,0,false),
ledLowFosso("ledLowFosso","led low fosso",persistance,0,false),
ledTorbidita("ledTorbidita","led torbidita",persistance,0,false),
ledGelo("ledGelo","led gelo",persistance,0,false),
ledSerbatoioVuoto("ledSerbatoioVuoto","led serbatoio vuoto",persistance,0,false),
ledErrorePompaFosso("ledErrorePompaFosso","led errore pompa fosso",persistance,0,false),
ledErrorePompaSerbatoio("ledErrorePompaSerbatoio","led errore pompa serbatoio",persistance,0,false)
{
    om.addObject(&device);
    om.addObject(&vbatt);
    om.addObject(&ppm);
    om.addObject(&trenchLevel);
    om.addObject(&storageLevel);
    om.addObject(&airTemperature);
    om.addObject(&waterTemperature);
    om.addObject(&pumpOnTime);
    om.addObject(&pumpDailyCycles);
    om.addObject(&svuotamentoCanaleTh);
    om.addObject(&riempimentoSerbatoioTh);
    om.addObject(&svuotamentoSerbatoioTh);
    om.addObject(&batteryGoodTh);
    om.addObject(&airTemperatureGoodTh);
    om.addObject(&ppmGoodTh);
    om.addObject(&trenchPumpCmd);
    om.addObject(&storagePumpCmd);
    om.addObject(&ledAlive);
    om.addObject(&ledLowBatt);
    om.addObject(&ledLowFosso);
    om.addObject(&ledTorbidita);
    om.addObject(&ledGelo);
    om.addObject(&ledSerbatoioVuoto);
    om.addObject(&ledErrorePompaFosso);
    om.addObject(&ledErrorePompaSerbatoio);

    ow.init(6);   
#ifndef LINUX_PLATFORM    
    mcp23x17Dev.port = I2C_NUM_0;
    mcp23x17Dev.addr = MCP23X17_ADDR_BASE;
    mcp23x17Dev.cfg.sda_io_num = GPIO_NUM_6;
    mcp23x17Dev.cfg.scl_io_num = GPIO_NUM_7;
    mcp23x17Dev.cfg.master.clk_speed = 125000;
    mcp23x17Dev.cfg.clk_flags = 0;

    i2c_dev_create_mutex(&mcp23x17Dev);
    mcp23x17_port_set_mode(&mcp23x17Dev,0x00FF); //port A->IN , portb-> out
    mcp23x17_port_set_pullup(&mcp23x17Dev,0xFF00);

   /* ads11xDev.port = I2C_NUM_0;
    ads11xDev.addr = ADS111X_ADDR_GND;
    ads11xDev.cfg.sda_io_num = GPIO_NUM_6;
    ads11xDev.cfg.scl_io_num = GPIO_NUM_7;
    ads11xDev.cfg.clk_flags = 0;
    ads11xDev.cfg.master.clk_speed = 125000;
    i2c_dev_create_mutex(&ads11xDev);
    ads111x_set_mode(&ads11xDev, ADS111X_MODE_SINGLE_SHOT); 
    ads111x_set_data_rate(&ads11xDev, ADS111X_DATA_RATE_8);
    ads111x_set_gain(&ads11xDev, ADS111X_GAIN_4V096);*/


    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << POMPA_FOSSO_GPIO) | (1ULL << POMPA_SERBATOIO_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);
#endif
    start();
}

DataManager::~DataManager()
{

}


string DataManager::getSSID()
{
    return device.getPropertyValueString(PROP_SSID);
}
string DataManager::getPwd()
{
    return device.getPropertyValueString(PROP_SSID_KEY);

}
string DataManager::getKey()
{
return device.getPropertyValueString(PROP_TS_KEY);
}

bool DataManager::adcRead(ads111x_mux_t mux, float &res)
{
    bool ret = false;
#ifdef LINUX_PLATFORM
    res =0.0f;
    return true;
#else
    ads111x_set_input_mux(&ads11xDev, mux);
    ads111x_start_conversion(&ads11xDev);
    for (int i = 0; i < 10; i++)
    {
        bool busy;
        ads111x_is_busy(&ads11xDev,&busy);
        if (!busy)
        {
            ret = true;
            break;
        }
        vTaskDelay(10);
    }
    int16_t val;
    ads111x_get_value(&ads11xDev, &val);
    res = val*4096.0f/32760.0f;
    return ret;
#endif
}

void DataManager::doInput()
{
    float tmpVal;
    bool res;

    // 1wire AIR temperatures;
    res = ow.readTemperature(0,tmpVal);
    airTemperature.setValue(tmpVal,!res);

    // 1wire WATER temperatures;
    res = ow.readTemperature(1,tmpVal);
    waterTemperature.setValue(tmpVal,!res);

    //analog input #0 (TDS sensor)
    res = adcRead(ADS111X_MUX_0_GND,tmpVal);
    float _ppm = 66.71f*pow(tmpVal,3) - 127.93f*pow(tmpVal,2) + 428.7f*tmpVal;
	_ppm = _ppm * (1.0f+0.02f*(waterTemperature.getValue() - 25.0f));
    ppm.setValue(_ppm,!res);

    //analog input #1 (battery voltage)
    res = adcRead(ADS111X_MUX_1_GND,tmpVal);
    vbatt.setValue(tmpVal,!res);

    // STORAGE LEVEL
    storageLevel.setValue(storageLevelMeter.getMeasure(),storageLevelMeter.getMeasure()>=5.9f);

    // TRENCH LEVEL
   // trenchLevel.setValue(trenchLevelMeter.getMeasure(),trenchLevelMeter.getMeasure()>=5.9f);

    // DEVICE
    device.setFreeHeap(hal.heapOccupation());

    // ROTARY SWITCHES
    uint16_t digital;
#ifdef LINUX_PLATFORM
    digital = 0;
#else
    mcp23x17_port_read(&mcp23x17Dev,&digital);
#endif
    pumpOnTime.setValue(digital & 0x0F);
    pumpDailyCycles.setValue((digital & 0xF0) >> 4);
}

void DataManager::doOutput()
{
    uint16_t out = 0;   
    if (ledAlive.getValue())
    {
        out |= 0x0100;
    }
    if (ledLowBatt.getValue())
    {
        out |= 0x0200;
    }
    if (ledLowFosso.getValue())
    {
        out |= 0x0400;
    }
    if (ledTorbidita.getValue())
    {
        out |= 0x0800;
    }
    if (ledGelo.getValue())
    {
        out |= 0x1000;
    }
    if (ledSerbatoioVuoto.getValue())
    {
        out |= 0x2000;
    }
    if (ledErrorePompaFosso.getValue())
    {
        out |= 0x4000;
    }
    if (ledErrorePompaSerbatoio.getValue())
    {
        out |= 0x8000;
    }
#ifndef LINUX_PLATFORM
    mcp23x17_port_write(&mcp23x17Dev,out);
    gpio_set_level(POMPA_SERBATOIO_GPIO,storagePumpCmd.getValue());
    gpio_set_level(POMPA_FOSSO_GPIO,trenchPumpCmd.getValue());
#endif
}

bool DataManager::pompa_on(int irrigazioni_al_giorno, int durata_secondi) {
    if (irrigazioni_al_giorno <= 0 || durata_secondi <= 0)
        return false;

    // Tempo totale in un giorno (secondi)
    int secondi_giorno = 24 * 60 * 60;

    // Intervallo tra una irrigazione e l'altra
    int intervallo = secondi_giorno / irrigazioni_al_giorno;

    // Ottieni l'orario corrente
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    int secondi_da_mezzanotte =
        t->tm_hour * 3600 +
        t->tm_min * 60 +
        t->tm_sec;

    // Controlla se siamo dentro una finestra di irrigazione
    for (int i = 0; i < irrigazioni_al_giorno; i++) {
        int inizio = i * intervallo;
        int fine = inizio + durata_secondi;

        if (secondi_da_mezzanotte >= inizio &&
            secondi_da_mezzanotte < fine) {
            return true; // pompa ON
        }
    }

    return false; // pompa OFF
}

void DataManager::loop()
{
    while (true)
    {
        msleep(1000);
        doInput();

        svuotamentoCanaleTh.setValue(trenchLevel.getValue());

        riempimentoSerbatoioTh.setValue(storageLevel.getValue());

        svuotamentoSerbatoioTh.setValue(storageLevel.getValue());

        batteryGoodTh.setValue(vbatt.getValue());

        airTemperatureGoodTh.setValue(airTemperature.getValue());

        ppmGoodTh.setValue(ppm.getValue());

        trenchPumpCmd.setValue(
            airTemperatureGoodTh.getValue() &&
            batteryGoodTh.getValue() &&
            svuotamentoCanaleTh.getValue() &&
            riempimentoSerbatoioTh.getValue()
        );

        storagePumpCmd.setValue(
            airTemperatureGoodTh.getValue() &&
            batteryGoodTh.getValue() &&
            svuotamentoSerbatoioTh.getValue() &&
            pompa_on(
                pumpDailyCycles.getValue(),
                pumpOnTime.getValue()*60
            )
        );

        ledAlive.setValue(!ledAlive.getValue());
        ledLowBatt.setValue(!batteryGoodTh.getValue());
        ledLowFosso.setValue(!svuotamentoCanaleTh.getValue());
        ledTorbidita.setValue(!ppmGoodTh.getValue());
        ledGelo.setValue(!airTemperatureGoodTh.getValue());
        ledSerbatoioVuoto.setValue(!svuotamentoSerbatoioTh.getValue());
        ledTorbidita.setValue(!ppmGoodTh.getValue());
        ledErrorePompaFosso.setValue(false);
        ledErrorePompaSerbatoio.setValue(false);
        doOutput();
    }
}