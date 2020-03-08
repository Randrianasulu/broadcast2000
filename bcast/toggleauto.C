#include <string.h>
#include "toggleauto.h"

ToggleAuto::ToggleAuto(ToggleAutos *autos)
 : Auto((Autos*)autos)
{
}

ToggleAuto::~ToggleAuto()
{
}

int ToggleAuto::value_to_str(char *string, float value)
{
		if(value > 0) sprintf(string, "ON");
		else sprintf(string, "OFF");
return 0;
}
