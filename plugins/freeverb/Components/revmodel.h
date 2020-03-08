// Reverb model declaration
//
// Written by Jezar at Dreampoint, June 2000
// http://www.dreampoint.co.uk
// This code is public domain

#ifndef _revmodel_
#define _revmodel_

#include "comb.h"
#include "allpass.h"

class revmodel
{
public:
	revmodel();
	void mute();
	void processmix(float *inputL, 
		float *inputR, 
		float *outputL, 
		float *outputR, 
		long numsamples, 
		int skip);
	void processreplace(float *inputL, 
		float *inputR, 
		float *outputL, 
		float *outputR, 
		long numsamples, 
		int skip);
	void setroomsize(float value);
	float getroomsize();
	void setdamp(float value);
	float getdamp();
	void setwet(float value);
	float getwet();
	void setdry(float value);
	float getdry();
	void setwidth(float value);
	float getwidth();
	void setmode(float value);
	float getmode();

	static int numcombs;	
	static int numallpasses;
	static float muted;  
	static float fixedgain;
	static float scalewet;	
	static float scaledry;	
	static float scaledamp;	
	static float scaleroom;	
	static float offsetroom;	
	static float initialroom;
	static float initialdamp;
	static float initialwet;	
	static float initialdry;	
	static float initialwidth;
	static float initialmode;
	static float freezemode;	
	static int stereospread;

// These values assume 44.1KHz sample rate
// they will probably be OK for 48KHz sample rate
// but would need scaling for 96KHz (or other) sample rates.
// The values were obtained by listening tests.
	static int combtuningL1;
	static int combtuningR1;
	static int combtuningL2;
	static int combtuningR2;
	static int combtuningL3;
	static int combtuningR3;
	static int combtuningL4;
	static int combtuningR4;
	static int combtuningL5;
	static int combtuningR5;
	static int combtuningL6;
	static int combtuningR6;
	static int combtuningL7;
	static int combtuningR7;
	static int combtuningL8;
	static int combtuningR8;
	static int allpasstuningL1;
	static int allpasstuningR1;
	static int allpasstuningL2;
	static int allpasstuningR2;
	static int allpasstuningL3;
	static int allpasstuningR3;
	static int allpasstuningL4;
	static int allpasstuningR4;

private:
			void	update();
private:
	float	gain;
	float	roomsize,roomsize1;
	float	damp,damp1;
	float	wet,wet1,wet2;
	float	dry;
	float	width;
	float	mode;

	// The following are all declared inline 
	// to remove the need for dynamic allocation
	// with its subsequent error-checking messiness

	// Comb filters
	comb	*combL;
	comb	*combR;

	// Allpass filters
	allpass	*allpassL;
	allpass	*allpassR;

	// Buffers for the combs
	float	*bufcombL1;
	float	*bufcombR1;
	float	*bufcombL2;
	float	*bufcombR2;
	float	*bufcombL3;
	float	*bufcombR3;
	float	*bufcombL4;
	float	*bufcombR4;
	float	*bufcombL5;
	float	*bufcombR5;
	float	*bufcombL6;
	float	*bufcombR6;
	float	*bufcombL7;
	float	*bufcombR7;
	float	*bufcombL8;
	float	*bufcombR8;

	// Buffers for the allpasses
	float	*bufallpassL1;
	float	*bufallpassR1;
	float	*bufallpassL2;
	float	*bufallpassR2;
	float	*bufallpassL3;
	float	*bufallpassR3;
	float	*bufallpassL4;
	float	*bufallpassR4;
};

#endif//_revmodel_

//ends
