#ifndef COLORS_H
#define COLORS_H

#include "vframe.inc"

class HSV
{
public:
	HSV();
    ~HSV();

	int rgb_to_hsv(float r, float g, float b, float &h, float &s, float &v);
	int hsv_to_rgb(float &r, float &g, float &b, float h, float s, float v);
};

class YUV
{
public:
	YUV();
	~YUV();

	inline void rgb_to_yuv(long &r, long &g, long &b, long &y, long &u, long &v)
	{
		y = (rtoy_tab[r] + gtoy_tab[g] + btoy_tab[b]) >> 8;
		u = (rtou_tab[r] + gtou_tab[g] + btou_tab[b]) >> 8;
		v = (rtov_tab[r] + gtov_tab[g] + btov_tab[b]) >> 8;
	};

	inline void yuv_to_rgb(long &r, long &g, long &b, long &y, long &u, long &v)
	{
		y <<= 8;
		r = (y + vtor[v]) >> 8;
		g = (y + utog[u] + vtog[v]) >> 8;
		b = (y + utob[u]) >> 8;

		if(r > VMAX) r = VMAX;
		else
		if(r < 0) r = 0;
		if(g > VMAX) g = VMAX;
		else
		if(g < 0) g = 0;
		if(b > VMAX) b = VMAX;
		else
		if(b < 0) b = 0;
	};

private:
	long rtoy_tab[VMAX + 1], gtoy_tab[VMAX + 1], btoy_tab[VMAX + 1];
	long rtou_tab[VMAX + 1], gtou_tab[VMAX + 1], btou_tab[VMAX + 1];
	long rtov_tab[VMAX + 1], gtov_tab[VMAX + 1], btov_tab[VMAX + 1];

	long vtor_tab[VMAX + 1], vtog_tab[VMAX + 1];
	long utog_tab[VMAX + 1], utob_tab[VMAX + 1];
	long *vtor, *vtog, *utog, *utob;
};

#endif
