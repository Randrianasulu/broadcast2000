#include "units.h"
#include <stdlib.h>

float* DB::topower = 0;
int* Freq::freqtable = 0;

DB::DB(float infinitygain)
{
	this->infinitygain = infinitygain;
	if(!topower)
	{
		int i;
		float value;

		// db to power table
		topower = new float[(MAXGAIN - INFINITYGAIN) * 10 + 1];
		topower += -INFINITYGAIN * 10;
		for(i = INFINITYGAIN * 10; i <= MAXGAIN * 10; i++)
		{
			topower[i] = pow(10, (float)i / 10 / 20);
			
//printf("%f %f\n", (float)i/10, topower[i]);
		}
		topower[INFINITYGAIN * 10] = 0;   // infinity gain
	}
	db = 0;
}

float DB::fromdb_table() 
{ 
	return db = topower[(int)(db * 10)]; 
}

float DB::fromdb_table(float db) 
{ 
	if(db > MAXGAIN) db = MAXGAIN;
	if(db <= INFINITYGAIN) return 0;
	return db = topower[(int)(db * 10)]; 
}

float DB::fromdb()
{
	return pow(10, db / 20);
}

float DB::fromdb(float db)
{
	return pow(10, db / 20);
}

// set db to the power given using a formula
float DB::todb(float power)
{
	float db;
	if(power == 0) 
		db = -100;
	else 
	{
		db = (float)(20 * log10(power));
		if(db < -100) db = -100;
	}
	return db;
}


Freq::Freq()
{
	if(!freqtable)
	{
  		int i;

		freqtable = new int[TOTALFREQS + 1];
  		double freq = 28;  // starting frequency
  		long scale = 60;   // some number divisable by three

  		freqtable[0] = 0;
  		for(i = 1; i <= TOTALFREQS; i++)
  		{
    		freqtable[i] = (int)freq;
    		freq = freq + freq / scale;
  		}
	}
	freq = 0;
}

Freq::Freq(const Freq& oldfreq)
{
	this->freq = oldfreq.freq;
}

int Freq::fromfreq() 
{
	int i;

  	for(i = 0; i < TOTALFREQS && freqtable[i] < freq; i++)
    	;
  	return(i);
};

int Freq::fromfreq(int index) 
{
	int i;

  	for(i = 0; i < TOTALFREQS && freqtable[i] < index; i++)
    	;
  	return(i);
};

int Freq::tofreq(int index) 
{ 
	freq = freqtable[index]; 
	return freq; 
}

Freq& Freq::operator++() 
{
	if(freq < TOTALFREQS) freq++;
	return *this;
}
	
Freq& Freq::operator--()
{
	if(freq > 0) freq--;
	return *this;
}
	
int Freq::operator>(Freq &newfreq) { return freq > newfreq.freq; }
int Freq::operator<(Freq &newfreq) { return freq < newfreq.freq; }
Freq& Freq::operator=(const Freq &newfreq) { freq = newfreq.freq; return *this; }
int Freq::operator=(const int newfreq) { freq = newfreq; return newfreq; }
int Freq::operator!=(Freq &newfreq) { return freq != newfreq.freq; }
int Freq::operator==(Freq &newfreq) { return freq == newfreq.freq; }
int Freq::operator==(int newfreq) { return freq == newfreq; }

char* Units::totext(char *text, 
			double seconds, 
			int time_format, 
			int sample_rate, 
			float frame_rate, 
			float frames_per_foot)    // give text representation as time
{
	int hour, minute;
	long frame, feet;

	switch(time_format)
	{
		case TIME_HMS:
		{
			float second;

  			hour = (int)(seconds / 3600);
  			minute = (int)(seconds / 60 - hour * 60);
  			second = (float)seconds - (long)hour * 3600 - (long)minute * 60;
  			sprintf(text, "%d:%d:%.3f", hour, minute, second);
			return text;
		}
		  break;
		
		case TIME_HMS2:
		{
			float second;

  			hour = (int)(seconds / 3600);
  			minute = (int)(seconds / 60 - hour * 60);
  			second = (float)seconds - (long)hour * 3600 - (long)minute * 60;
  			sprintf(text, "%d:%02d:%02d", hour, minute, (int)second);
			return text;
		}
		  break;

		case TIME_HMSF:
		{
			int second;

  			hour = (int)(seconds / 3600);
  			minute = (int)(seconds / 60 - hour * 60);
  			second = (int)(seconds - hour * 3600 - minute * 60);
  			frame = (long)(frame_rate * 
  	 			 (float)((float)seconds - (long)hour * 3600 - (long)minute * 60 - second))
  	 			 ;
  			sprintf(text, "%01d:%02d:%02d:%02ld", hour, minute, second, frame);
			return text;
		}
			break;
			
		case TIME_SAMPLES:
  			sprintf(text, "%ld", (long)(seconds * sample_rate + 0.5));
			break;
		
		case TIME_SAMPLES_HEX:
  			sprintf(text, "%x", (long)(seconds * sample_rate + 0.5));
			break;
		
		case TIME_FRAMES:
			frame = (long)(seconds * frame_rate + 0.5);
			sprintf(text, "%ld", frame);
			return text;
			break;
		
		case TIME_FEET_FRAMES:
			frame = (long)(seconds * frame_rate + 0.5);
			feet = (long)(frame / frames_per_foot);
			sprintf(text, "%ld-%ld", feet, (long)(frame - feet * frames_per_foot));
			return text;
			break;
	}
	return text;
}


char* Units::totext(char *text, 
		long samples, 
		int samplerate, 
		int time_format, 
		float frame_rate,
		float frames_per_foot)
{
	return totext(text, (double)samples / samplerate, time_format, samplerate, frame_rate, frames_per_foot);
}    // give text representation as time

longest Units::fromtext(char *text, 
			int samplerate, 
			int time_format, 
			float frame_rate,
			float frames_per_foot)
{
	longest hours, minutes, frames, total_samples, i, j;
	long feet;
	float seconds;
	char string[256];
	
	switch(time_format)
	{
		case 0:
// get hours
			i = 0;
			j = 0;
			while(text[i] >=48 && text[i] <= 57 && j < 10) string[j++] = text[i++];
			string[j] = 0;
			hours = atol(string);
// get minutes
			j = 0;
// skip separator
			while((text[i] < 48 || text[i] > 57) && text[i] != 0) i++;
			while(text[i] >=48 && text[i] <= 57 && j < 10) string[j++] = text[i++];
			string[j] = 0;
			minutes = atol(string);
			
// get seconds
			j = 0;
// skip separator
			while((text[i] < 48 || text[i] > 57) && text[i] != 0) i++;
			while((text[i] == '.' || (text[i] >=48 && text[i] <= 57)) && j < 10) string[j++] = text[i++];
			string[j] = 0;
			seconds = atof(string);

			total_samples = (unsigned long)(((double)seconds + minutes * 60 + hours * 3600) * samplerate);
			return total_samples;
			break;

		case 1:
// get hours
			i = 0;
			j = 0;
			while(text[i] >=48 && text[i] <= 57 && j < 10) string[j++] = text[i++];
			string[j] = 0;
			hours = atol(string);
			
// get minutes
			j = 0;
// skip separator
			while((text[i] < 48 || text[i] > 57) && text[i] != 0) i++;
			while(text[i] >=48 && text[i] <= 57 && j < 10) string[j++] = text[i++];
			string[j] = 0;
			minutes = atol(string);
			
// get seconds
			j = 0;
// skip separator
			while((text[i] < 48 || text[i] > 57) && text[i] != 0) i++;
			while(text[i] >=48 && text[i] <= 57 && j < 10) string[j++] = text[i++];
			string[j] = 0;
			seconds = atof(string);
			
// skip separator
			while((text[i] < 48 || text[i] > 57) && text[i] != 0) i++;
// get frames
			j = 0;
			while(text[i] >=48 && text[i] <= 57 && j < 10) string[j++] = text[i++];
			string[j] = 0;
			frames = atol(string);
			
			total_samples = (long)(((float)frames / frame_rate + seconds + minutes*60 + hours*3600) * samplerate);
			return total_samples;
			break;

		case 2:
			return atol(text);
			break;
		
		case 3:
			sscanf(text, "%x", &total_samples);
			return total_samples;
		
		case 4:
			return (long)(atof(text) / frame_rate * samplerate);
			break;
		
		case 5:
// Get feet
			i = 0;
			j = 0;
			while(text[i] >=48 && text[i] <= 57 && text[i] != 0 && j < 10) string[j++] = text[i++];
			string[j] = 0;
			feet = atol(string);

// skip separator
			while((text[i] < 48 || text[i] > 57) && text[i] != 0) i++;

// Get frames
			j = 0;
			while(text[i] >=48 && text[i] <= 57 && text[i] != 0 && j < 10) string[j++] = text[i++];
			string[j] = 0;
			frames = atol(string);
			return (long)(((float)feet * frames_per_foot + frames) / frame_rate * samplerate);
			break;
	}
	return 0;
}

double Units::text_to_seconds(char *text, 
				int samplerate, 
				int time_format, 
				float frame_rate, 
				float frames_per_foot)
{
	return (double)fromtext(text, 
		samplerate, 
		time_format, 
		frame_rate, 
		frames_per_foot) / samplerate;
}






float Units::toframes(long samples, int sample_rate, float framerate) 
{ 
	return (float)samples / sample_rate * framerate; 
} // give position in frames

long Units::toframes_round(long samples, int sample_rate, float framerate) 
{
// used in editing
	float result_f = (float)samples / sample_rate * framerate; 
	long result_l = (long)(result_f + 0.5);
	return result_l;
}

long Units::tosamples(float frames, int sample_rate, float framerate) 
{ 
	float result = (frames / framerate * sample_rate);
	
	if(result - (int)result) result += 1;
	return (long)result;
} // give position in samples


float Units::xy_to_polar(int x, int y)
{
	float angle = 0;
	if(x > 0 && y <= 0)
	{
		angle = atan((float)-y / x) / (2 * M_PI) * 360;
	}
	else
	if(x < 0 && y <= 0)
	{
		angle = 180 - atan((float)-y / -x) / (2 * M_PI) * 360;
	}
	else
	if(x < 0 && y > 0)
	{
		angle = 180 - atan((float)-y / -x) / (2 * M_PI) * 360;
	}
	else
	if(x > 0 && y > 0)
	{
		angle = 360 + atan((float)-y / x) / (2 * M_PI) * 360;
	}
	else
	if(x == 0 && y < 0)
	{
		angle = 90;
	}
	else
	if(x == 0 && y > 0)
	{
		angle = 270;
	}
	else
	if(x == 0 && y == 0)
	{
		angle = 0;
	}

	return angle;
}

void Units::polar_to_xy(float angle, int radius, int &x, int &y)
{
	while(angle < 0) angle += 360;

	x = (int)(cos(angle / 360 * (2 * M_PI)) * radius);
	y = (int)(-sin(angle / 360 * (2 * M_PI)) * radius);
}

long Units::round(double result)
{
	return (long)(result < 0 ? result - 0.5 : result + 0.5);
}

float Units::quantize10(float value)
{
	long temp = (long)(value * 10 + 0.5);
	value = (float)temp / 10;
	return value;
}





