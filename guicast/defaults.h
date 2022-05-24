#ifndef DEFAULTS_H
#define DEFAULTS_H

#include "units.h"

class Defaults
{
public:
	Defaults(char *filename);
	virtual ~Defaults();

	int load();        // load from disk file
	int save();        // save to disk file
	int update(const char *name, Freq value); // update a value if it exists
	int update(const char *name, double value); // update a value if it exists
	int update(const char *name, float value); // update a value if it exists
	int update(const char *name, int value); // update a value if it exists
	int update(const char *name, long value); // update a value if it exists
	int update(const char *name, char *value); // create it if it doesn't

	double get(char *name, double default_);   // retrieve a value if it exists
	float get(char *name, float default_);   // retrieve a value if it exists
	int get(char *name, int default_);   // retrieve a value if it exists
	long get(char *name, long default_);   // retrieve a value if it exists
	char* get(char *name, char *default_); // return 1 if it doesn't
	
	char *names[1024];  // list of string names
	char *values[1024];    // list of values
	int total;             // number of defaults
	char filename[1024];        // filename the defaults are stored in
};

#endif
