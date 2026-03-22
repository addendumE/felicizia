#pragma once

#include "Thread.h"

using namespace std;


class UsRange: public Thread 
{
    public:
        UsRange(int com_port,int rxPin):Thread("th"){};
        virtual ~UsRange(){};
        float getMeasure(){return 0.0f;};
    private:
    void loop(){sleep(1);};
};
