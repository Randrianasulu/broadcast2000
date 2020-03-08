#include <string.h>
#include "floatauto.h"

FloatAuto::FloatAuto(FloatAutos *autos)
 : Auto((Autos*)autos)
{
}

FloatAuto::~FloatAuto()
{
}


int FloatAuto::value_to_str(char *string, float value)
{
	int j = 0, i = 0;
	if(value > 0) sprintf(string, "+%.2f", value);
	else sprintf(string, "%.2f", value);

// fix number
	if(value == 0)
	{
		j = 0;
		string[1] = 0;
	}
	else
	if(value < 1 && value > -1) 
	{
		j = 1;
		string[j] = string[0];
	}
	else 
	{
		j = 0;
		string[3] = 0;
	}
	
	while(string[j] != 0) string[i++] = string[j++];
	string[i] = 0;
return 0;
}
