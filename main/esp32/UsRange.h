#pragma once

#include "Thread.h"
#include "Lock.h"
#include "driver/uart.h"

using namespace std;


class UsRange: public Thread, public Lock
{
    public:
        UsRange(int com_port,int rxPin);
        virtual ~UsRange();
        bool getMeasure(float &);
    private:
    uart_port_t com_port;
    uart_config_t uart_config;
    float distance;
    bool fail;
    void loop();
};
