#ifndef FLOATAUTO_H
#define FLOATAUTO_H

// Automation point that takes floating point values

class FloatAuto;

#include "auto.h"
#include "floatautos.inc"

class FloatAuto : public Auto
{
public:
	FloatAuto() {};
	FloatAuto(FloatAutos *autos);
	~FloatAuto();

private:
	int value_to_str(char *string, float value);
};



#endif
