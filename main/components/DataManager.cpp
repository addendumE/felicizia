#include "DataManager.h"
#include <math.h>
#include <stdint.h>
#include <esp_log.h>
static const char *TAG="DM";
//#define IICdebug

#define POMPA_SERBATOIO_GPIO    GPIO_NUM_8
#define POMPA_FOSSO_GPIO        GPIO_NUM_5
#define IN_BOOT_BUTTON          GPIO_NUM_9
#define IN_FEEDBACK_P_CANALE    GPIO_NUM_21
#define IN_FEEDBACK_P_SERBATOIO    GPIO_NUM_20

DataManager::DataManager (ObjManager &om,Persistance &persistance,Hal &hal):
Thread("dataManager"),
om(om),
#if !defined(LINUX_PLATFORM) && !defined(IICdebug)
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
livFosso("livFosso","livello fosso",persistance,UNIT_DISTANCE,2),
livSerbatoio("livSerbatoio","livello serbatoio",persistance,UNIT_DISTANCE,2),
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
orarioOk("orarioOk","orario ok",persistance,UNIT_HOURS,2,false,Threshold::MODE_IN),
cmdPompaRiempimento("cmdPriemp","comando pompa riempimento",persistance,false),
cmdPompaIrrigazione("cmdPirrig","comando pompa irrigazione",persistance,false),
ledAlive("ledAlive","led alive",persistance,false),
ledLowBatt("ledLowBatt","led batteria bassa",persistance,false),
ledLowFosso("ledLowFosso","led fosso vuoto",persistance,false),
ledTorbidita("ledTorb","led torbidita anomala",persistance,false),
ledLowTaria("ledLowTaria","led temperatura aria bassa",persistance,false),
ledLowSerbatoio("ledLowSerb","led serbatoio vuoto",persistance,false),
ledErrorePompaFosso("ledErrPfosso","led errore pompa fosso",persistance,false),
ledErrorePompaSerbatoio("ledErrPserb","led errore pompa serbatoio",persistance,false),
feedbackPompaCanale("feedPcanale","feedback pompa canale",persistance),
feedbackPompaSerbatoio("feddPserb","feedback pompa serbatoio",persistance),
last_state(1),
loopCnt(0),
oldOut({})
{
    oldOut.boot = true;
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
    om.addObject(&orarioOk);
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
    om.addObject(&feedbackPompaCanale);
    om.addObject(&feedbackPompaSerbatoio);
#if !defined(LINUX_PLATFORM) && !defined(IICdebug)
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
    ads111x_set_gain(&ads11xDev, ADS111X_GAIN_2V048);


    gpio_config_t out_conf = {
        .pin_bit_mask = (1ULL << POMPA_FOSSO_GPIO) | (1ULL << POMPA_SERBATOIO_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&out_conf);
    ow.init(GPIO_NUM_2);
    gpio_config_t in_conf = {};
    in_conf.intr_type = GPIO_INTR_DISABLE;
    in_conf.mode = GPIO_MODE_INPUT;
    in_conf.pin_bit_mask = (1ULL << IN_BOOT_BUTTON) | (1ULL << IN_FEEDBACK_P_CANALE) | (1ULL << IN_FEEDBACK_P_SERBATOIO);
    in_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    in_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpio_config(&in_conf);
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

string DataManager::getMqttUri()
{
return device.getPropertyValueString(PROP_MQTT_URI);
}

string DataManager::getUid()
{
return device.getPropertyValueString(PROP_UID);
}

bool DataManager::adcRead(ads111x_mux_t mux, float &res)
{
    bool ret = false;
#if defined(LINUX_PLATFORM) || defined(IICdebug)
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
        res = (val*2048.0f/32760.0f)/1000.0f;
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

    //analog input #1 (TDS sensor)
    res = adcRead(ADS111X_MUX_1_GND,tmpVal);
    float _ppm = 66.71f*pow(tmpVal,3) - 127.93f*pow(tmpVal,2) + 428.7f*tmpVal;
	_ppm = _ppm * (1.0f+0.02f*(tempAcqua.getValue() - 25.0f));
    sccm.setValue(_ppm,!res);

    //analog input #0 (battery voltage)
    res = adcRead(ADS111X_MUX_0_GND,tmpVal);
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
#if !defined(LINUX_PLATFORM) && !defined(IICdebug)
    mcp23x17_port_read(&mcp23x17Dev,&digital);
    ESP_LOGI(TAG,"%04X",digital);
#endif
    tempoIrrigazione.setValue(digital & 0x07);
    numeroIrrigazioni.setValue((digital >> 4)& 0x0F);
    feedbackPompaCanale.setValue(gpio_get_level(IN_FEEDBACK_P_CANALE));
    feedbackPompaSerbatoio.setValue(gpio_get_level(IN_FEEDBACK_P_SERBATOIO));
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
#if !defined(LINUX_PLATFORM) && !defined(IICdebug)
    mcp23x17_port_write(&mcp23x17Dev,out);
    gpio_set_level(POMPA_SERBATOIO_GPIO,cmdPompaIrrigazione.getValue());
    gpio_set_level(POMPA_FOSSO_GPIO,cmdPompaRiempimento.getValue());

    int state = gpio_get_level(IN_BOOT_BUTTON);
    if (state != last_state && state == 0) {
        if (bootButtonCallBack) bootButtonCallBack();
    }
    last_state = state;
#endif    
    

}

bool DataManager::pompa_on(int irrigazioni_al_giorno,
                           int durata_secondi,
                           float start,
                           float end)
{
    if (irrigazioni_al_giorno <= 0 || durata_secondi <= 0)
    {
        ESP_LOGW(TAG, "Configurazione errata %d %d",
                 irrigazioni_al_giorno, durata_secondi);
        return false;
    }

    // Orario corrente
    time_t now = time(nullptr);
    struct tm *t = localtime(&now);

    int secondi_da_mezzanotte =
        t->tm_hour * 3600 +
        t->tm_min * 60 +
        t->tm_sec;

    // Conversione ore -> secondi
    int start_sec = static_cast<int>(start * 3600.0f);
    int end_sec   = static_cast<int>(end * 3600.0f);

    // La finestra deve essere valida e non attraversare la mezzanotte
    if (start_sec >= end_sec)
    {
        ESP_LOGW(TAG, "Finestra oraria non valida");
        return false;
    }

    // Fuori dalla finestra di irrigazione
    if (secondi_da_mezzanotte < start_sec ||
        secondi_da_mezzanotte >= end_sec)
    {
        ESP_LOGI(TAG, "POMPA OFF - fuori finestra");
        return false;
    }

    int durata_finestra = end_sec - start_sec;
    int intervallo = durata_finestra / irrigazioni_al_giorno;

    // Evita divisioni per zero
    if (intervallo <= 0)
    {
        ESP_LOGW(TAG, "Intervallo non valido");
        return false;
    }

    int tempo_relativo = secondi_da_mezzanotte - start_sec;
    int offset_nello_slot = tempo_relativo % intervallo;

    bool on = (offset_nello_slot < durata_secondi);

    ESP_LOGI(TAG, "POMPA %s, %d %d", on ? "ON" : "OFF", offset_nello_slot, durata_secondi);

    return on;
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


        // Ottieni l'orario corrente
        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        float ore_da_mezzanotte = t->tm_hour + (t->tm_min / 60.0f) + (t->tm_sec / 3600.0f);
        orarioOk.setValue(ore_da_mezzanotte);

        cmdPompaRiempimento.setValue(
            temperaturaAriaOk.getValue() &&
            tensioneBatteriaOk.getValue() &&
            livSvuotamentoCanaleOk.getValue() &&
            livRiempimentoSerbatoioOk.getValue() &&
            orarioOk.getValue() &&
            !cmdPompaIrrigazione.getValue()
        );

        ESP_LOGI(TAG,"TA:%d VB:%d LIV:%d OR:%d",
        temperaturaAriaOk.getValue(),
        tensioneBatteriaOk.getValue(),
        livSvuotamentoCanaleOk.getValue(),
        orarioOk.getValue()
        );
        float start,end;
        orarioOk.getThValue(end,start);
        cmdPompaIrrigazione.setValue(
            temperaturaAriaOk.getValue() &&
            tensioneBatteriaOk.getValue() &&
            livIrrigazioneOk.getValue() &&
            orarioOk.getValue() &&

            pompa_on(
                (int)numeroIrrigazioni.getValue(),
                (int)tempoIrrigazione.getValue()*60,
                start,
                end
            )
        );

        ledAlive.setValue(!ledAlive.getValue());
        ledLowBatt.setValue(!tensioneBatteriaOk.getValue());
        ledLowFosso.setValue(!livSvuotamentoCanaleOk.getValue());
        ledLowTaria.setValue(!temperaturaAriaOk.getValue());
        ledLowSerbatoio.setValue(!livIrrigazioneOk.getValue());
        ledTorbidita.setValue(!conducibilitaOk.getValue());
        ledErrorePompaFosso.setValue(cmdPompaRiempimento.getFail());
        ledErrorePompaSerbatoio.setValue(cmdPompaIrrigazione.getFail());
        doOutput();
        if (++loopCnt % 600 == 0)
        {
            ts.clean();
            ts.setValue(1,vbatt.getValue());
            ts.setValue(2,livFosso.getValue());
            ts.setValue(3,livSerbatoio.getValue());
            ts.setValue(4,tempAria.getValue());
            ts.setValue(5,tempAcqua.getValue());
            ts.setValue(6,sccm.getValue());
            ts.publish(getKey());
        }
        else {
            string s = "";
            if (oldOut.boot)
            {
                s = s+ " boot";
                oldOut.boot = false;
            }
            if (oldOut.ledLowBatt!=ledLowBatt.getValue())
            {
                s = s+ " ledLowBatt="+ (ledLowBatt.getValue() ? "ON":"OFF");
                oldOut.ledLowBatt=ledLowBatt.getValue();
            }
            if (oldOut.ledLowFosso!=ledLowFosso.getValue())
            {
                s = s+ " ledLowFosso="+ (ledLowFosso.getValue() ? "ON":"OFF");
                oldOut.ledLowFosso=ledLowFosso.getValue();
            }
            if (oldOut.ledLowTaria!=ledLowTaria.getValue())
            {
                s = s+ " ledLowTaria="+ (ledLowTaria.getValue() ? "ON":"OFF");
                oldOut.ledLowTaria=ledLowTaria.getValue();
            }
            if (oldOut.ledLowSerbatoio!=ledLowSerbatoio.getValue())
            {
                s = s+ " ledLowSerbatoio="+ (ledLowSerbatoio.getValue() ? "ON":"OFF");
                oldOut.ledLowSerbatoio=ledLowSerbatoio.getValue();
            }
            if (oldOut.ledTorbidita!=ledTorbidita.getValue())
            {
                s = s+ " ledTorbidita="+ (ledTorbidita.getValue() ? "ON":"OFF");
                oldOut.ledTorbidita=ledTorbidita.getValue();
            }
            if (oldOut.ledErrorePompaFosso!=ledErrorePompaFosso.getValue())
            {
                s = s+ " ledErrorePompaFosso="+ (ledErrorePompaFosso.getValue() ? "ON":"OFF");
                oldOut.ledErrorePompaFosso=ledErrorePompaFosso.getValue();
            }
            if (oldOut.ledErrorePompaSerbatoio!=ledErrorePompaSerbatoio.getValue())
            {
                s = s+ " ledErrorePompaSerbatoio="+ (ledErrorePompaSerbatoio.getValue() ? "ON":"OFF");
                oldOut.ledErrorePompaSerbatoio=ledErrorePompaSerbatoio.getValue();
            }
            if (oldOut.cmdPompaIrrigazione!=cmdPompaIrrigazione.getValue())
            {
                s = s+ " cmdPompaIrrigazione="+ (cmdPompaIrrigazione.getValue() ? "ON":"OFF");
                oldOut.cmdPompaIrrigazione=cmdPompaIrrigazione.getValue();
            }
            if (oldOut.cmdPompaRiempimento!=cmdPompaRiempimento.getValue())
            {
                s = s+ " cmdPompaRiempimento="+ (cmdPompaRiempimento.getValue() ? "ON":"OFF");
                oldOut.cmdPompaRiempimento=cmdPompaRiempimento.getValue();
            }
            if (s.size())
            {
                ts.clean();
                ts.setValue(1,vbatt.getValue());
                ts.setValue(2,livFosso.getValue());
                ts.setValue(3,livSerbatoio.getValue());
                ts.setValue(4,tempAria.getValue());
                ts.setValue(5,tempAcqua.getValue());
                ts.setValue(6,sccm.getValue());
                ts.setValue(7,s);
                ts.publish(getKey());
            }
            
        }
    }
}

void DataManager::setDeviceStatus(const string &status)
{
    device.setPropertyValueString(Types::PROP_VALUE,status);
}
