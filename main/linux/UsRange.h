#pragma once

#include "Thread.h"

using namespace std;


class UsRange: public Thread 
{
    public:
        UsRange(int com_port,int rxPin):Thread("th"){};
        virtual ~UsRange(){};
        bool getMeasure(float &val ){return true;};
    private:
    void loop(){sleep(1);};
};
