#include "Adc.h"

Adc::Adc() {

}

Adc::~Adc() {

}
#include <stdlib.h>
float Adc::readVoltage(int ch)
{
	float ret;
	take();
	ret = (3.3f*rand()/RAND_MAX);
	give();
	return ret;
}

