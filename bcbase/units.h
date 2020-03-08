#ifndef UNITS_H
#define UNITS_H

#include <math.h>
#include <stdio.h>
#define INFINITYGAIN -40
#define MAXGAIN 50
#define TOTALFREQS 400

// time formats:	// 0 HMS   
					// 1 HMSFrames
					// 2 integer samples
					// 3 hex samples
					// 4 frames count
					// 5 feet-frames

class DB
{
public:
	DB(float infinitygain = INFINITYGAIN);
	virtual ~DB() {  };
	
// return power of db using a table
	float fromdb();
	float fromdb(float db);

// set db to the power given using a formula
	float todb(float newdb);

	inline DB& operator++() { if(db < MAXGAIN) db += 0.1; return *this; };
	inline DB& operator--() { if(db > INFINITYGAIN) db -= 0.1; return *this; };
	inline int operator=(DB &newdb) { db = newdb.db; return 0; };
	inline DB& operator=(int newdb) { db = newdb; return *this; };
	inline int operator==(DB &newdb) { return db == newdb.db; };
	inline int operator==(int newdb) { return db == newdb; };

	static float *topower;
	float db;
	float infinitygain;
};

class Freq
{
public:
	Freq();
	Freq(const Freq& oldfreq);
	virtual ~Freq() {  };

// set freq to index given
	int tofreq(int index);

// return index of frequency
	int fromfreq();
	int fromfreq(int index);

// increment frequency by one
	Freq& operator++();
	Freq& operator--();
	
	int operator>(Freq &newfreq);
	int operator<(Freq &newfreq);
	Freq& operator=(const Freq &newfreq);
	int operator=(const int newfreq);
	int operator!=(Freq &newfreq);
	int operator==(Freq &newfreq);
	int operator==(int newfreq);

	static int *freqtable;
	int freq;
};



// No rounding.
float toframes(long samples, int sample_rate, float framerate);
// Round up if > .5
long toframes_round(long samples, int sample_rate, float framerate);

long tosamples(float frames, int sample_rate, float framerate);
char* totext(char *text, 
			long samples, 
			int samplerate, 
			int time_format, 
			float frame_rate, 
			float frames_per_foot);    // give text representation as time
long fromtext(char *text, 
			int samplerate, 
			int time_format, 
			float frame_rate, 
			float frames_per_foot);    // convert time to samples

#endif
