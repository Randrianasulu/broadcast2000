#include "colors.h"

#include <stdio.h>

HSV::HSV()
{
}


HSV::~HSV()
{
}



int HSV::rgb_to_hsv(float r, float g, float b, float &h, float &s, float &v)
{
    	int i;
	float min, max, delta;
	float f, p, q, t;
	min = ((r < g) ? r : g) < b ? ((r < g) ? r : g) : b;
	max = ((r > g) ? r : g) > b ? ((r > g) ? r : g) : b;
	v = max;                               // v

	delta = max - min;

//printf("HSV::rgb_to_hsv %f %f %f\n", max, min, delta);
	if(max != 0 && delta != 0)
        s = delta / max;               // s
	else 
	{
        // r = g = b = 0                // s = 0, v is undefined
        s = 0;
        h = -1;
        return 0;
	}

	if(r == max)
        h = (g - b) / delta;         // between yellow & magenta
	else 
	if(g == max)
        h = 2 + (b - r) / delta;     // between cyan & yellow
	else
        h = 4 + (r - g) / delta;     // between magenta & cyan

	h *= 60;                               // degrees
	if(h < 0)
        h += 360;
return 0;
}

int HSV::hsv_to_rgb(float &r, float &g, float &b, float h, float s, float v)
{
     	int i;
	float min, max, delta;
	float f, p, q, t;
    if(s == 0) 
	{
        // achromatic (grey)
        r = g = b = v;
        return 0;
    }

    h /= 60;                        // sector 0 to 5
    i = (int)h;
    f = h - i;                      // factorial part of h
    p = v * (1 - s);
    q = v * (1 - s * f);
    t = v * (1 - s * (1 - f));

    switch(i) 
	{
        case 0:
            r = v;
            g = t;
            b = p;
            break;
        case 1:
            r = q;
            g = v;
            b = p;
            break;
        case 2:
            r = p;
            g = v;
            b = t;
            break;
        case 3:
            r = p;
            g = q;
            b = v;
            break;
        case 4:
            r = t;
            g = p;
            b = v;
            break;
        default:                // case 5:
            r = v;
            g = p;
            b = q;
            break;
    }
return 0;
}


YUV::YUV()
{
	for(int i = 0; i < VMAX + 1; i++)
	{
// compression
		rtoy_tab[i] = (long)( 0.2990 * 256 * i);
		rtou_tab[i] = (long)(-0.1687 * 256 * i);
		rtov_tab[i] = (long)( 0.5000 * 256 * i);

		gtoy_tab[i] = (long)( 0.5870 * 256 * i);
		gtou_tab[i] = (long)(-0.3320 * 256 * i);
		gtov_tab[i] = (long)(-0.4187 * 256 * i);

		btoy_tab[i] = (long)( 0.1140 * 256 * i);
		btou_tab[i] = (long)( 0.5000 * 256 * i);
		btov_tab[i] = (long)(-0.0813 * 256 * i);
	}

	vtor = &(vtor_tab[(VMAX + 1) / 2]);
	vtog = &(vtog_tab[(VMAX + 1) / 2]);
	utog = &(utog_tab[(VMAX + 1) / 2]);
	utob = &(utob_tab[(VMAX + 1) / 2]);

	for(int i = (-VMAX - 1) / 2; i < (VMAX + 1) / 2; i++)
	{
// decompression
		vtor[i] = (long)( 1.4020 * 256 * i);
		vtog[i] = (long)(-0.7141 * 256 * i);

		utog[i] = (long)(-0.3441 * 256 * i);
		utob[i] = (long)( 1.7720 * 256 * i);
	}
}

YUV::~YUV()
{
}
