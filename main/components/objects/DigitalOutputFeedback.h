#pragma once
#include "DigitalOutput.h"
class DigitalOutputFeedback:public DigitalOutput {
public:
	DigitalOutputFeedback(string id, string name, Persistance & p , bool _default);
	virtual ~DigitalOutputFeedback();
	void setValue(bool val);
	void setFeedbackValue(bool val);
	bool getFail(){return fail;};
private:
	time_t  tOn;
	bool oldVal;
	bool fail;
};
