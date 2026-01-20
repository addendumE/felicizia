#pragma once
class Ads1115
{
public:
    Ads1115(int busNumber, int devAddress){};
    virtual ~Ads1115(){};
    typedef enum { // register address
        CONVERSION_REGISTER_ADDR = 0,
        CONFIG_REGISTER_ADDR,
    } registers;
    typedef enum { // multiplex options
        MUX_0_1 = 0,
        MUX_0_3,
        MUX_1_3,
        MUX_2_3,
        MUX_0_GND,
        MUX_1_GND,
        MUX_2_GND,
        MUX_3_GND,
    } mux;

    typedef enum { // full-scale resolution options
        FSR_6_144 = 0,
        FSR_4_096,
        FSR_2_048,
        FSR_1_024,
        FSR_0_512,
        FSR_0_256,
    } fsr;

    typedef enum { // samples per second
        SPS_8 = 0,
        SPS_16,
        SPS_32,
        SPS_64,
        SPS_128,
        SPS_250,
        SPS_475,
        SPS_860
    } sps;

    typedef enum {
        MODE_CONTINUOUS = 0,
        MODE_SINGLE
    } mode_t;

    bool init (sps _sps)
    {
        return true;
    }

    bool read(float &out, int ch, fsr scale )
    {
        out = (float) ch;
        return true;
    }
private:
    int busNumber;
    int devAddres;
};