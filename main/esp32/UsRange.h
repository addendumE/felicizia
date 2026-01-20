#pragma once

#include "Thread.h"
#include "driver/uart.h"

using namespace std;


class UsRange: public Thread 
{
    public:
        UsRange(int com_port,int rxPin);
        virtual ~UsRange();
        float getMeasure();
    private:
    uart_port_t com_port;
    uart_config_t uart_config;
    float distance;
    void loop();
};
