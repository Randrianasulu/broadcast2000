#ifndef OVERLAYFRAME_H
#define OVERLAYFRAME_H

class OverlayPixelBase;

#include "overlayframe.inc"
#include "vframe.inc"

class OverlayFrame
{
public:
	OverlayFrame(int use_alpha, int use_float, int interpolate, int mode, int color_model = OVERLAY_VFRAME);
	virtual ~OverlayFrame();

	int compare_with(int use_float, int use_alpha, int interpolate, int mode);
// Alpha is from 0 - VMAX
	int overlay(VFrame *output, VFrame *input,
		float in_x1, float in_y1, float in_x2, float in_y2,
		float out_x1, float out_y1, float out_x2, float out_y2, 
		int alpha);
	int overlay(VFrame *output, unsigned char *input,
		float in_x1, float in_y1, float in_x2, float in_y2,
		float out_x1, float out_y1, float out_x2, float out_y2, 
		int alpha, int in_w, int in_h);
	int use_alpha, use_float, mode, interpolate;
	int color_model;

private:
	int transfer_direct(VFrame *output, VFrame *input_v, unsigned char *input_c, 
		int in_x1, int in_y1, int in_x2, int in_y2,
		int out_x1, int out_y1, int out_x2, int out_y2, 
		int alpha, int in_w, int in_h);

// Transfer without interpolation
	int transfer_scale_i(VFrame *output, VFrame *input, unsigned char *input_c, 
		int in_x1, int in_y1, int in_x2, int in_y2,
		int out_x1, int out_y1, int out_x2, int out_y2, 
		int alpha, int in_w, int in_h);

// Transfer with interpolation
	int transfer_scale_f(VFrame *output, VFrame *input_v, unsigned char *input_c, 
		float in_x1, float in_y1, float in_x2, float in_y2,
		float out_x1, float out_y1, float out_x2, float out_y2, 
		int alpha, int in_w, int in_h);

// Get tables for scaling without interpolation
	int get_scale_array(int *column_table, int *row_table, 
		int in_x1, int in_y1, int in_x2, int in_y2,
		int out_x1, int out_y1, int out_x2, int out_y2);

// Gat tables for enlarge with interpolation
	int get_enlarge_array(int *input_pixel1, float *pixel1_fraction, 
		int *input_pixel2, float *pixel2_fraction,
		float *total_fraction, 
		float in_pixel1, float in_pixel2, float out_pixel1, float out_pixel2, 
		int out_total, int in_total);

// Get tables for reduce with interpolation.
// Each output pixel has an input pixel range.
// Input fractions 1 and 3 are for boundary pixels in the range.
// Between the boundaries, all pixels use fraction 2.
	int get_reduce_array(int *input_pixel1, int *input_pixel2, 
		float *input_fraction1, float *input_fraction2, float *input_fraction3, 
		float *total_fraction, 
		float in_pixel1, float in_pixel2, float out_pixel1, float out_pixel2, 
		int out_total, int in_total);

	int transfer_row_direct(VPixel *output, VPixel *input, int out_columns, 
		int alpha);
	int transfer_row_direct(VPixel *output, unsigned char *input, int out_columns, 
		int alpha);
	int transfer_row_scale(VPixel *output, VPixel *input, int out_columns, int *column_table, 
		int alpha);
	int transfer_row_scale(VPixel *output, unsigned char *input, int out_columns, int *column_table, 
		int alpha);

// Overlay pixel for interpolation

	int interpolate_pixel_enlarge(VPixel &output, VPixel &input1, VPixel &input2, VPixel &input3, VPixel &input4,
		float &fraction1, float &fraction2, float &fraction3, float &fraction4, float &alpha);
	int interpolate_pixel_enlarge(VPixel &output, 
		unsigned char *input1, unsigned char *input2, unsigned char *input3, unsigned char *input4,
		float &fraction1, float &fraction2, float &fraction3, float &fraction4, 
		float total_fraction, 
		float &alpha, int &in_w, int &in_h);

	int interpolate_pixel_reduce(VPixel &output, VPixel **input, 
		int &xinput_pixel1, int &xinput_pixel2, 
		float &xinput_fraction1, float &xinput_fraction2, float &xinput_fraction3, 
		int &yinput_pixel1, int &yinput_pixel2, 
		float &yinput_fraction1, float &yinput_fraction2, float &yinput_fraction3, 
		float &a_float);
	int interpolate_pixel_reduce(VPixel &output, unsigned char *input, 
		int &xinput_pixel1, int &xinput_pixel2, 
		float &xinput_fraction1, float &xinput_fraction2, float &xinput_fraction3, 
		int &yinput_pixel1, int &yinput_pixel2, 
		float &yinput_fraction1, float &yinput_fraction2, float &yinput_fraction3, 
		float total_fraction, 
		float &a_float, int &in_w, int &in_h);

// Used by interpolate_pixel_ routines
	VPixel temp;   // Temporary pixel used by interpolation
	unsigned char temp_c[5];  // Temporary pixel used by interpolation
	float temp_f[5];

	OverlayPixelBase *pixel_overlay;
	VPixel *input_row, *input_end;
	unsigned char *input_row_c, *input_end_c;
	float input_scale1, input_scale2, input_scale3;
	int depth; // Bytes per pixel
	int i, j;
};

class OverlayPixelBase
{
public:
	OverlayPixelBase();
	virtual ~OverlayPixelBase();

	virtual int overlay_pixel_f(VPixel &output, VPixel &input, float &alpha);
	virtual int overlay_pixel_i(VPixel &output, VPixel &input, int &alpha);
	virtual int overlay_pixel_f(VPixel &output, unsigned char *input, float &alpha);
	virtual int overlay_pixel_i(VPixel &output, unsigned char *input, int &alpha);
	virtual int overlay_unsigned_f(VPixel &output, unsigned char *input, float &opacity, int alpha);

	float input_opacity_f, output_opacity_f;
	long input_opacity_i, output_opacity_i;
	long r_i, g_i, b_i, a_i;
	float r_f, g_f, b_f, a_f;
};

class OverlayPixelNormalAlpha : public OverlayPixelBase
{
public:
	OverlayPixelNormalAlpha();
	int overlay_pixel_f(VPixel &output, VPixel &input, float &alpha);
	int overlay_pixel_i(VPixel &output, VPixel &input, int &alpha);
};

class OverlayPixelNormal : public OverlayPixelBase
{
public:
	OverlayPixelNormal();
	int overlay_pixel_f(VPixel &output, VPixel &input, float &alpha);
	int overlay_pixel_i(VPixel &output, VPixel &input, int &alpha);
};

class OverlayPixelAdditionAlpha : public OverlayPixelBase
{
public:
	OverlayPixelAdditionAlpha();
	int overlay_pixel_f(VPixel &output, VPixel &input, float &alpha);
	int overlay_pixel_i(VPixel &output, VPixel &input, int &alpha);
};

class OverlayPixelAddition : public OverlayPixelBase
{
public:
	OverlayPixelAddition();
	int overlay_pixel_f(VPixel &output, VPixel &input, float &alpha);
	int overlay_pixel_i(VPixel &output, VPixel &input, int &alpha);
};

class OverlayPixelSubtractAlpha : public OverlayPixelBase
{
public:
	OverlayPixelSubtractAlpha();
	int overlay_pixel_f(VPixel &output, VPixel &input, float &alpha);
	int overlay_pixel_i(VPixel &output, VPixel &input, int &alpha);
};

class OverlayPixelSubtract : public OverlayPixelBase
{
public:
	OverlayPixelSubtract();
	int overlay_pixel_f(VPixel &output, VPixel &input, float &alpha);
	int overlay_pixel_i(VPixel &output, VPixel &input, int &alpha);
};

class OverlayPixelMultiplyAlpha : public OverlayPixelBase
{
public:
	OverlayPixelMultiplyAlpha();
	int overlay_pixel_f(VPixel &output, VPixel &input, float &alpha);
	int overlay_pixel_i(VPixel &output, VPixel &input, int &alpha);
};

class OverlayPixelMultiply : public OverlayPixelBase
{
public:
	OverlayPixelMultiply();
	int overlay_pixel_f(VPixel &output, VPixel &input, float &alpha);
	int overlay_pixel_i(VPixel &output, VPixel &input, int &alpha);
};

class OverlayPixelDivideAlpha : public OverlayPixelBase
{
public:
	OverlayPixelDivideAlpha();
	int overlay_pixel_f(VPixel &output, VPixel &input, float &alpha);
	int overlay_pixel_i(VPixel &output, VPixel &input, int &alpha);
};


class OverlayPixelDivide : public OverlayPixelBase
{
public:
	OverlayPixelDivide();
	int overlay_pixel_f(VPixel &output, VPixel &input, float &alpha);
	int overlay_pixel_i(VPixel &output, VPixel &input, int &alpha);
};

// Various color models

class OverlayPixelAlphaRGB : public OverlayPixelBase
{
public:
	OverlayPixelAlphaRGB();
	int overlay_pixel_f(VPixel &output, unsigned char *input, float &alpha);
	int overlay_pixel_i(VPixel &output, unsigned char *input, int &alpha);
	int overlay_unsigned_f(VPixel &output, unsigned char *input, float &opacity, int alpha);
};

class OverlayPixelRGB : public OverlayPixelBase
{
public:
	OverlayPixelRGB();
	int overlay_pixel_f(VPixel &output, unsigned char *input, float &alpha);
	int overlay_pixel_i(VPixel &output, unsigned char *input, int &alpha);
	int overlay_unsigned_f(VPixel &output, unsigned char *input, float &opacity, int alpha);
};

class OverlayPixelAlphaRGBA : public OverlayPixelBase
{
public:
	OverlayPixelAlphaRGBA();
	int overlay_pixel_f(VPixel &output, unsigned char *input, float &alpha);
	int overlay_pixel_i(VPixel &output, unsigned char *input, int &alpha);
	int overlay_unsigned_f(VPixel &output, unsigned char *input, float &opacity, int alpha);
};

class OverlayPixelRGBA : public OverlayPixelBase
{
public:
	OverlayPixelRGBA();
	int overlay_pixel_f(VPixel &output, unsigned char *input, float &alpha);
	int overlay_pixel_i(VPixel &output, unsigned char *input, int &alpha);
	int overlay_unsigned_f(VPixel &output, unsigned char *input, float &opacity, int alpha);
};

#endif
