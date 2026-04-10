#include "DataManager.h"
#include <math.h>
#include <stdint.h>
static const char *TAG="DM";

#define POMPA_SERBATOIO_GPIO    GPIO_NUM_8
#define POMPA_FOSSO_GPIO        GPIO_NUM_5
#define IN_BOOT_BUTTON          GPIO_NUM_9

DataManager::DataManager (ObjManager &om,Persistance &persistance,Hal &hal):
Thread("dataManager"),
om(om),
#ifndef LINUX_PLATFORM
mcp23x17Dev({}),
ads11xDev({}),
#endif
persistance(persistance),
hal(hal),
livSerbatoioMeter(0,4),
livFossoMeter(1,3),
device("device","device",persistance),
vbatt("vbatt","tensione batteria",persistance,UNIT_VOLTAGE,1),
sccm("sccm","conducibilità",persistance,UNIT_CONDUCTIVITY,1),
livFosso("livFosso","livello fosso",persistance,UNIT_DISTANCE,1),
livSerbatoio("livSerbatoio","livello serbatoio",persistance,UNIT_DISTANCE,1),
tempAria("tempAria","temperatura aria",persistance,UNIT_TEMPERATURE,1),
tempAcqua("tempAcqua","temperatura acqua",persistance,UNIT_TEMPERATURE,1),
tempoIrrigazione("tempoIrrig","tempo irrigazione",persistance,UNIT_MINUTES,1),
numeroIrrigazioni("numIrrig","numero irrigazioni",persistance,UNIT_NONE,0),
livSvuotamentoCanaleOk("livSvtFossoOk","fosso pronto a svuotamento",persistance,UNIT_DISTANCE,1,false,Threshold::MODE_OVER),
livRiempimentoSerbatoioOk("livRmpSerbOk","serbatoio pronto a riempimento",persistance,UNIT_DISTANCE,1,false,Threshold::MODE_UNDER),
livIrrigazioneOk("livIrrigOk","serbatoio pronto a svuotamento",persistance,UNIT_DISTANCE,1,false,Threshold::MODE_OVER),
tensioneBatteriaOk("vBattOk","batteria ok",persistance,UNIT_VOLTAGE,1,false,Threshold::MODE_OVER),
temperaturaAriaOk("tempAriaOk","temperatura aria ok",persistance,UNIT_TEMPERATURE,1,false,Threshold::MODE_OVER),
conducibilitaOk("sccmOk","conducibilità ok",persistance,UNIT_CONDUCTIVITY,1,false,Threshold::MODE_IN),
cmdPompaRiempimento("cmdPriemp","comando pompa riempimento",persistance,0,false),
cmdPompaIrrigazione("cmdPirrig","comando pompa irrigazione",persistance,0,false),
ledAlive("ledAlive","led alive",persistance,0,false),
ledLowBatt("ledLowBatt","led batteria bassa",persistance,0,false),
ledLowFosso("ledLowFosso","led fosso vuoto",persistance,0,false),
ledTorbidita("ledTorb","led torbidita anomala",persistance,0,false),
ledLowTaria("ledLowTaria","led temperatura aria bassa",persistance,0,false),
ledLowSerbatoio("ledLowSerb","led serbatoio vuoto",persistance,0,false),
ledErrorePompaFosso("ledErrPfosso","led errore pompa fosso",persistance,0,false),
ledErrorePompaSerbatoio("ledErrPserb","led errore pompa serbatoio",persistance,0,false),
last_state(1)
{
    om.addObject(&device);
    om.addObject(&vbatt);
    om.addObject(&sccm);
    om.addObject(&livFosso);
    om.addObject(&livSerbatoio);
    om.addObject(&tempAria);
    om.addObject(&tempAcqua);
    om.addObject(&tempoIrrigazione);
    om.addObject(&numeroIrrigazioni);
    om.addObject(&livSvuotamentoCanaleOk);
    om.addObject(&livRiempimentoSerbatoioOk);
    om.addObject(&livIrrigazioneOk);
    om.addObject(&tensioneBatteriaOk);
    om.addObject(&temperaturaAriaOk);
    om.addObject(&conducibilitaOk);
    om.addObject(&cmdPompaRiempimento);
    om.addObject(&cmdPompaIrrigazione);
    om.addObject(&ledAlive);
    om.addObject(&ledLowBatt);
    om.addObject(&ledLowFosso);
    om.addObject(&ledTorbidita);
    om.addObject(&ledLowTaria);
    om.addObject(&ledLowSerbatoio);
    om.addObject(&ledErrorePompaFosso);
    om.addObject(&ledErrorePompaSerbatoio);
#ifndef LINUX_PLATFORM    
    mcp23x17Dev.port = I2C_NUM_0;
    mcp23x17Dev.addr = MCP23X17_ADDR_BASE;
    mcp23x17Dev.cfg.sda_io_num = GPIO_NUM_6;
    mcp23x17Dev.cfg.scl_io_num = GPIO_NUM_7;
    mcp23x17Dev.cfg.master.clk_speed = 125000;
    mcp23x17Dev.cfg.clk_flags = 0;

    i2c_dev_create_mutex(&mcp23x17Dev);
    mcp23x17_port_set_mode(&mcp23x17Dev,0x00FF); //port A->IN , portb-> out
    mcp23x17_port_set_pullup(&mcp23x17Dev,0xFFFF);

    ads11xDev.port = I2C_NUM_0;
    ads11xDev.addr = ADS111X_ADDR_GND;
    ads11xDev.cfg.sda_io_num = GPIO_NUM_6;
    ads11xDev.cfg.scl_io_num = GPIO_NUM_7;
    ads11xDev.cfg.clk_flags = 0;
    ads11xDev.cfg.master.clk_speed = 125000;
    i2c_dev_create_mutex(&ads11xDev);
    ads111x_set_mode(&ads11xDev, ADS111X_MODE_SINGLE_SHOT); 
    ads111x_set_data_rate(&ads11xDev, ADS111X_DATA_RATE_8);
    ads111x_set_gain(&ads11xDev, ADS111X_GAIN_4V096);


    gpio_config_t out_conf = {
        .pin_bit_mask = (1ULL << POMPA_FOSSO_GPIO) | (1ULL << POMPA_SERBATOIO_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&out_conf);
    ow.init(GPIO_NUM_2);
#endif
    gpio_config_t in_conf = {};
    in_conf.intr_type = GPIO_INTR_DISABLE;
    in_conf.mode = GPIO_MODE_INPUT;
    in_conf.pin_bit_mask = (1ULL << IN_BOOT_BUTTON);
    in_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    in_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpio_config(&in_conf);
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
    if (ret)
    {
        int16_t val = 0;
        ads111x_get_value(&ads11xDev, &val);
        res = val*4096.0f/32760.0f;
    }
    else
    {
        ESP_LOGW(TAG,"ads11xDev timeout");
    }
    return ret;
#endif
}

void DataManager::doInput()
{
    float tmpVal;
    bool res;

    // 1wire AIR temperatures;
    res = ow.readTemperature(0,tmpVal);
    tempAria.setValue(tmpVal,!res);

    // 1wire WATER temperatures;
    res = ow.readTemperature(1,tmpVal);
    tempAcqua.setValue(tmpVal,!res);

    //analog input #0 (TDS sensor)
    res = adcRead(ADS111X_MUX_0_GND,tmpVal);
    float _ppm = 66.71f*pow(tmpVal,3) - 127.93f*pow(tmpVal,2) + 428.7f*tmpVal;
	_ppm = _ppm * (1.0f+0.02f*(tempAcqua.getValue() - 25.0f));
    sccm.setValue(_ppm,!res);

    //analog input #1 (battery voltage)
    res = adcRead(ADS111X_MUX_1_GND,tmpVal);
    vbatt.setValue(tmpVal,!res);

    // STORAGE LEVEL
    bool fail = livSerbatoioMeter.getMeasure(tmpVal);
    livSerbatoio.setValue(tmpVal ,tmpVal>=5.9f || fail);

    // TRENCH LEVEL
    fail = livFossoMeter.getMeasure(tmpVal);
    livFosso.setValue(tmpVal,tmpVal>=5.9f || fail);

    // DEVICE
    device.setFreeHeap(hal.heapOccupation());

    // ROTARY SWITCHES
    uint16_t digital = 0;
#ifndef LINUX_PLATFORM
    mcp23x17_port_read(&mcp23x17Dev,&digital);
    ESP_LOGI(TAG,"%04X",digital);
#endif
    tempoIrrigazione.setValue(digital & 0x07);
    numeroIrrigazioni.setValue((digital >> 4)& 0x0F);
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
    if (ledLowTaria.getValue())
    {
        out |= 0x1000;
    }
    if (ledLowSerbatoio.getValue())
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
    gpio_set_level(POMPA_SERBATOIO_GPIO,cmdPompaIrrigazione.getValue());
    gpio_set_level(POMPA_FOSSO_GPIO,cmdPompaRiempimento.getValue());
#endif
    int state = gpio_get_level(IN_BOOT_BUTTON);
    if (state != last_state && state == 0) {
        if (bootButtonCallBack) bootButtonCallBack();
    }
    last_state = state;
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

        livSvuotamentoCanaleOk.setValue(livFosso.getValue());

        livRiempimentoSerbatoioOk.setValue(livSerbatoio.getValue());

        livIrrigazioneOk.setValue(livSerbatoio.getValue());

        tensioneBatteriaOk.setValue(vbatt.getValue());

        temperaturaAriaOk.setValue(tempAria.getValue());

        conducibilitaOk.setValue(sccm.getValue());

        cmdPompaRiempimento.setValue(
            temperaturaAriaOk.getValue() &&
            tensioneBatteriaOk.getValue() &&
            livSvuotamentoCanaleOk.getValue() &&
            livRiempimentoSerbatoioOk.getValue()
        );

        cmdPompaIrrigazione.setValue(
            temperaturaAriaOk.getValue() &&
            tensioneBatteriaOk.getValue() &&
            livIrrigazioneOk.getValue() &&
            pompa_on(
                (int)numeroIrrigazioni.getValue(),
                (int)tempoIrrigazione.getValue()*60
            )
        );

        ledAlive.setValue(!ledAlive.getValue());
        ledLowBatt.setValue(!tensioneBatteriaOk.getValue());
        ledLowFosso.setValue(!livSvuotamentoCanaleOk.getValue());
        ledTorbidita.setValue(!conducibilitaOk.getValue());
        ledLowTaria.setValue(!temperaturaAriaOk.getValue());
        ledLowSerbatoio.setValue(!livIrrigazioneOk.getValue());
        ledTorbidita.setValue(!conducibilitaOk.getValue());
        ledErrorePompaFosso.setValue(false);
        ledErrorePompaSerbatoio.setValue(false);
        doOutput();
    }
}