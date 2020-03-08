#ifndef TOGGLEAUTO_H
#define TOGGLEAUTO_H

// Automation point that takes floating point values

#include "auto.h"
#include "toggleautos.inc"

class ToggleAuto : public Auto
{
public:
	ToggleAuto() {};
	ToggleAuto(ToggleAutos *autos);
	~ToggleAuto();

private:
	int value_to_str(char *string, float value);
};



#endif
