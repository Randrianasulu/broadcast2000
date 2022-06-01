#include <string.h>
#include <stdlib.h>
#include "defaults.h"
#include "filesystem.h"
#include "stringfile.h"

Defaults::Defaults(const char *filename)
{
	strcpy(this->filename, filename);
	FileSystem directory;
	
	directory.parse_tildas(this->filename);
	total = 0;
}

Defaults::~Defaults()
{
	for(int i = 0; i < total; i++)
	{
		delete names[i];
		delete values[i];
	}
}

int Defaults::load()
{
	char arg1[1024], arg2[1024];
	
	StringFile stringfile(filename);
	total = 0;
	while(stringfile.get_pointer() < stringfile.get_length())
	{
		stringfile.readline(arg1, arg2);
		names[total] = new char[strlen(arg1) + 1];
		values[total] = new char[strlen(arg2) + 1];
		strcpy(names[total], arg1);
		strcpy(values[total], arg2);
		total++;
	}
return 0;
}

int Defaults::save()
{
	StringFile stringfile;
	for(int i = 0; i < total; i++)
	{
		stringfile.writeline(names[i], values[i], 0);
	}
	stringfile.write_to_file(filename);
return 0;
}

int Defaults::get(const char *name, int default_)
{
	for(int i = 0; i < total; i++)
	{
		if(!strcmp(names[i], name))
		{
			return (int)atol(values[i]);
		}
	}
	return default_;  // failed
return 0;
}

long Defaults::get(const char *name, long default_)
{
	for(int i = 0; i < total; i++)
	{
		if(!strcmp(names[i], name))
		{
			return atol(values[i]);
		}
	}
	return default_;  // failed
}

float Defaults::get(const char *name, float default_)
{
	for(int i = 0; i < total; i++)
	{
		if(!strcmp(names[i], name))
		{
			return atof(values[i]);
		}
	}
	return default_;  // failed
}

char* Defaults::get(const char *name, char *default_)
{
	for(int i = 0; i < total; i++)
	{
		if(!strcmp(names[i], name))
		{
			strcpy(default_, values[i]);
			return values[i];
		}
	}
	return default_;  // failed
}

int Defaults::update(const char *name, float value) // update a value if it exists
{
	char string[1024];
	sprintf(string, "%f", value);
	return update(name, string);
return 0;
}

int Defaults::update(const char *name, int value) // update a value if it exists
{
	char string[1024];
	sprintf(string, "%d", value);
	return update(name, string);
return 0;
}

int Defaults::update(const char *name, long value) // update a value if it exists
{
	char string[1024];
	sprintf(string, "%ld", value);
	return update(name, string);
return 0;
}

int Defaults::update(const char *name, char *value)
{
	for(int i = 0; i < total; i++)
	{
		if(!strcmp(names[i], name))
		{
			delete values[i];
			values[i] = new char[strlen(value) + 1];
			strcpy(values[i], value);
			return 0;
		}
	}
// didn't find so create new entry
	names[total] = new char[strlen(name) + 1];
	strcpy(names[total], name);
	values[total] = new char[strlen(value) + 1];
	strcpy(values[total], value);
	total++;
	return 1;
return 0;
}
