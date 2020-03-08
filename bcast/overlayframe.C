#include <string.h>
#include <math.h>
#include <stdio.h>
#include "overlayframe.h"
#include "vframe.h"

OverlayFrame::OverlayFrame(int use_alpha, int use_float, int interpolate, int mode, int color_model)
{
	this->use_alpha = use_alpha;
	this->use_float = use_float;
	this->interpolate = interpolate;
	this->color_model = color_model;
	this->mode = mode;
	pixel_overlay = 0;

	if(color_model == OVERLAY_VFRAME)
	{
		if(use_alpha)
		{
			switch(mode)
			{
				case NORMAL:   pixel_overlay = new OverlayPixelNormalAlpha;   break;
				case ADDITION: pixel_overlay = new OverlayPixelAdditionAlpha; break;
				case SUBTRACT: pixel_overlay = new OverlayPixelSubtractAlpha; break;
				case MULTIPLY: pixel_overlay = new OverlayPixelMultiplyAlpha; break;
				case DIVIDE:   pixel_overlay = new OverlayPixelDivideAlpha;   break;
			}
		}
		else
		{
			switch(mode)
			{
				case NORMAL:   pixel_overlay = new OverlayPixelNormal;   break;
				case ADDITION: pixel_overlay = new OverlayPixelAddition; break;
				case SUBTRACT: pixel_overlay = new OverlayPixelSubtract; break;
				case MULTIPLY: pixel_overlay = new OverlayPixelMultiply; break;
				case DIVIDE:   pixel_overlay = new OverlayPixelDivide;   break;
			}
		}
	}
	else
	{
		if(use_alpha)
		{
			switch(color_model)
			{
				case OVERLAY_RGB:
					pixel_overlay = new OverlayPixelAlphaRGB;
					depth = 3;
					break;

				case OVERLAY_RGBA:
					pixel_overlay = new OverlayPixelAlphaRGBA;
					depth = 4;
					break;
			}
		}
		else
		{
			switch(color_model)
			{
				case OVERLAY_RGB:
					pixel_overlay = new OverlayPixelRGB;
					depth = 3;
					break;

				case OVERLAY_RGBA:
					pixel_overlay = new OverlayPixelRGBA;
					depth = 4;
					break;
			}
		}
	}
}

OverlayFrame::~OverlayFrame()
{
	if(pixel_overlay) delete pixel_overlay;
}

int OverlayFrame::compare_with(int use_float, int use_alpha, int interpolate, int mode)
{
	if(this->use_float == use_float &&
		this->use_alpha == use_alpha &&
		this->mode == mode &&
		this->interpolate == interpolate)
		return 1;
	else
		return 0;
return 0;
}


int OverlayFrame::overlay(VFrame *output, VFrame *input, 
		float in_x1, float in_y1, float in_x2, float in_y2,
		float out_x1, float out_y1, float out_x2, float out_y2, 
		int alpha)
{
	float zoom_x, zoom_y;
	zoom_x = (float)(out_x2 - out_x1) / (in_x2 - in_x1);
	zoom_y = (float)(out_y2 - out_y1) / (in_y2 - in_y1);

	if(zoom_x == 1 && zoom_y == 1)
	{
		transfer_direct(output, input, 0, in_x1, in_y1, in_x2, in_y2,
			out_x1, out_y1, out_x2, out_y2, alpha, input->get_w(), input->get_h());
	}
	else
	{
		if(interpolate)
			transfer_scale_f(output, input, 0, in_x1, in_y1, in_x2, in_y2,
				out_x1, out_y1, out_x2, out_y2, alpha, input->get_w(), input->get_h());
		else
			transfer_scale_i(output, input, 0, (int)in_x1, (int)in_y1, (int)in_x2, (int)in_y2,
				(int)out_x1, (int)out_y1, (int)out_x2, (int)out_y2, alpha, input->get_w(), input->get_h());
	}
return 0;
}

int OverlayFrame::overlay(VFrame *output, unsigned char *input, 
		float in_x1, float in_y1, float in_x2, float in_y2,
		float out_x1, float out_y1, float out_x2, float out_y2, 
		int alpha, int in_w, int in_h)
{
	float zoom_x, zoom_y;
	zoom_x = (float)(out_x2 - out_x1) / (in_x2 - in_x1);
	zoom_y = (float)(out_y2 - out_y1) / (in_y2 - in_y1);

	if(zoom_x == 1 && zoom_y == 1)
	{
		transfer_direct(output, 0, input, in_x1, in_y1, in_x2, in_y2,
						out_x1, out_y1, out_x2, out_y2, alpha, in_w, in_h);
	}
	else
	{
		if(interpolate)
			transfer_scale_f(output, 0, input, in_x1, in_y1, in_x2, in_y2,
							out_x1, out_y1, out_x2, out_y2, alpha, in_w, in_h);
		else
			transfer_scale_i(output, 0, input, (int)in_x1, (int)in_y1, (int)in_x2, (int)in_y2,
							(int)out_x1, (int)out_y1, (int)out_x2, (int)out_y2, alpha, in_w, in_h);
	}
return 0;
}

int OverlayFrame::transfer_direct(VFrame *output, VFrame *input_v, unsigned char *input_c, 
		int in_x1, int in_y1, int in_x2, int in_y2,
		int out_x1, int out_y1, int out_x2, int out_y2, 
		int alpha, int in_w, int in_h)
{
	if(input_v)
	{
		while(in_y1 < in_y2)
		{
			transfer_row_direct(&(((VPixel**)output->get_rows())[out_y1][out_x1]), 
				&(((VPixel**)input_v->get_rows())[in_y1][in_x1]), 
				in_x2 - in_x1, alpha);

			in_y1++;
			out_y1++;
		}
	}
	else
	if(input_c)
	{
		while(in_y1 < in_y2)
		{
			transfer_row_direct(&(((VPixel**)output->get_rows())[out_y1][out_x1]), 
				&(input_c[in_y1 * in_w * depth + in_x1 * depth]), 
				in_x2 - in_x1, alpha);

			in_y1++;
			out_y1++;
		}
	}
return 0;
}

int OverlayFrame::transfer_scale_i(VFrame *output, VFrame *input_v, unsigned char *input_c, 
		int in_x1, int in_y1, int in_x2, int in_y2,
		int out_x1, int out_y1, int out_x2, int out_y2, 
		int alpha, int in_w, int in_h)
{
	int *column_table, *row_table;
	int y_out, h_out = out_y2 - out_y1;

	column_table = new int[out_x2 - out_x1];
	row_table = new int[out_y2 - out_y1];
	get_scale_array(column_table, row_table, 
			in_x1, in_y1, in_x2, in_y2,
			out_x1, out_y1, out_x2, out_y2);

	if(input_v)
	{
		for(y_out = 0; y_out < h_out; y_out++)
		{
			transfer_row_scale(&(((VPixel**)output->get_rows())[out_y1][out_x1]), 
				&(((VPixel**)input_v->get_rows())[row_table[y_out]][in_x1]), 
				out_x2 - out_x1, column_table, 
				alpha);
			out_y1++;
		}
	}
	else if(input_c)
	{
		for(y_out = 0; y_out < h_out; y_out++)
		{
			transfer_row_scale(&(((VPixel**)output->get_rows())[out_y1][out_x1]), 
				&(input_c[row_table[y_out] * in_w * depth + in_x1 * depth]), 
				out_x2 - out_x1, column_table, 
				alpha);
			out_y1++;
		}
	}
	delete [] column_table;
	delete [] row_table;
	
return 0;
}


int OverlayFrame::transfer_scale_f(VFrame *output, VFrame *input_v, unsigned char *input_c, 
		float in_x1, float in_y1, float in_x2, float in_y2,
		float out_x1, float out_y1, float out_x2, float out_y2, 
		int alpha, int in_w, int in_h)
{
	VPixel **output_rows = (VPixel**)output->get_rows();
	VPixel **input_rows;
	float a_float = (float)alpha / VMAX;
	float fraction1, fraction2, fraction3, fraction4;
	int i, out_y1_i, out_y2_i, out_x1_i, out_x2_i;
	float *total_x_fraction;
	float *total_y_fraction;

	total_x_fraction = new float[output->get_w() + 1];
	total_y_fraction = new float[output->get_h() + 1];

	out_y1_i = (int)(out_y1);
	out_x1_i = (int)(out_x1);
	out_y2_i = (int)(out_y2);
	out_x2_i = (int)(out_x2);

	if(input_v)
	{
		input_rows = (VPixel**)input_v->get_rows();
	}

	if(in_x2 - in_x1 < out_x2 - out_x1 && in_y2 - in_y1 < out_y2 - out_y1)
	{
// ============= optimized code for enlarging both dimensions ===================================
		int *xinput_pixel1;
		int *xinput_pixel2;
		float *xinput_fraction1;
		float *xinput_fraction2;
		int *yinput_pixel1;
		int *yinput_pixel2;
		float *yinput_fraction1;
		float *yinput_fraction2;
		register int y_out, x_out, h_out, w_out;
		
		xinput_pixel1 = new int[output->get_w() + 1];
		xinput_pixel2 = new int[output->get_w() + 1];
		xinput_fraction1 = new float[output->get_w() + 1];
		xinput_fraction2 = new float[output->get_w() + 1];
		yinput_pixel1 = new int[output->get_h() + 1];
		yinput_pixel2 = new int[output->get_h() + 1];
		yinput_fraction1 = new float[output->get_h() + 1];
		yinput_fraction2 = new float[output->get_h() + 1];

		h_out = output->get_h();
		w_out = output->get_w();

		get_enlarge_array(xinput_pixel1, xinput_fraction1, 
			xinput_pixel2, xinput_fraction2, 
			total_x_fraction, 
			in_x1, in_x2, out_x1, out_x2, 
			output->get_w(), in_w);
		get_enlarge_array(yinput_pixel1, yinput_fraction1, 
			yinput_pixel2, yinput_fraction2, 
			total_y_fraction, 
			in_y1, in_y2, out_y1, out_y2, 
			output->get_h(), in_h);

		if(input_v)
		{
			VPixel blank = {0, 0, 0, 0};
			VPixel *pixel1, *pixel2, *pixel3, *pixel4;
			for(y_out = out_y1_i; y_out < out_y2_i; y_out++)
			{
				for(x_out = out_x1_i; x_out < out_x2_i; x_out++)
				{
					fraction1 = xinput_fraction1[x_out] * yinput_fraction1[y_out];
					fraction2 = xinput_fraction1[x_out] * yinput_fraction2[y_out];
					fraction3 = xinput_fraction2[x_out] * yinput_fraction1[y_out];
					fraction4 = xinput_fraction2[x_out] * yinput_fraction2[y_out];

					interpolate_pixel_enlarge(output_rows[y_out][x_out], 
						(VPixel&)(fraction1 ? input_rows[yinput_pixel1[y_out]][xinput_pixel1[x_out]] : blank), 
						(VPixel&)(fraction2 ? input_rows[yinput_pixel2[y_out]][xinput_pixel1[x_out]] : blank), 
						(VPixel&)(fraction3 ? input_rows[yinput_pixel1[y_out]][xinput_pixel2[x_out]] : blank), 
						(VPixel&)(fraction4 ? input_rows[yinput_pixel2[y_out]][xinput_pixel2[x_out]] : blank),
						fraction1, fraction2, fraction3, fraction4, 
						a_float);
				}
			}
		}
		else
		if(input_c)
		{
			unsigned char blank_c[4];
			unsigned char *pixel1, *pixel2, *pixel3, *pixel4;
			for(y_out = out_y1_i; y_out < out_y2_i; y_out++)
			{
				for(x_out = out_x1_i; x_out < out_x2_i; x_out++)
				{
					fraction1 = xinput_fraction1[x_out] * yinput_fraction1[y_out];
					fraction2 = xinput_fraction1[x_out] * yinput_fraction2[y_out];
					fraction3 = xinput_fraction2[x_out] * yinput_fraction1[y_out];
					fraction4 = xinput_fraction2[x_out] * yinput_fraction2[y_out];

					if(fraction1)
						pixel1 = &input_c[yinput_pixel1[y_out] * in_w * depth + xinput_pixel1[x_out] * depth];
					else
						pixel1 = blank_c;

					if(fraction2)
						pixel2 = &input_c[yinput_pixel2[y_out] * in_w * depth + xinput_pixel1[x_out] * depth];
					else
						pixel2 = blank_c;

					if(fraction3)
						pixel3 = &input_c[yinput_pixel1[y_out] * in_w * depth + xinput_pixel2[x_out] * depth];
					else
						pixel3 = blank_c;

					if(fraction4)
				    	pixel4 = &input_c[yinput_pixel2[y_out] * in_w * depth + xinput_pixel2[x_out] * depth];
					else
						pixel4 = blank_c;

					interpolate_pixel_enlarge(output_rows[y_out][x_out], 
						pixel1, pixel2, pixel3, pixel4,
						fraction1, fraction2, fraction3, fraction4, 
						fraction1 + fraction2 + fraction3 + fraction4, 
						a_float, in_w, in_h);
				}
			}
		}
		delete [] xinput_pixel1;
		delete [] xinput_pixel2;
		delete [] xinput_fraction1;
		delete [] xinput_fraction2;
		delete [] yinput_pixel1;
		delete [] yinput_pixel2;
		delete [] yinput_fraction1;
		delete [] yinput_fraction2;
	}
	else
	{
// ============================== all other scaling =====================================
		int *xinput_pixel1;
		int *xinput_pixel2;
		int *yinput_pixel1;
		int *yinput_pixel2;
		float *xinput_fraction1;
		float *xinput_fraction2;
		float *xinput_fraction3;
		float *yinput_fraction1;
		float *yinput_fraction2;
		float *yinput_fraction3;
		register int y_out, x_out, h_out, w_out, i;

		xinput_pixel1 = new int[output->get_w() + 1];
		xinput_pixel2 = new int[output->get_w() + 1];
		yinput_pixel1 = new int[output->get_h() + 1];
		yinput_pixel2 = new int[output->get_h() + 1];
		xinput_fraction1 = new float[output->get_w() + 1];
		xinput_fraction2 = new float[output->get_w() + 1];
		xinput_fraction3 = new float[output->get_w() + 1];
		yinput_fraction1 = new float[output->get_h() + 1];
		yinput_fraction2 = new float[output->get_h() + 1];
		yinput_fraction3 = new float[output->get_h() + 1];

		h_out = output->get_h();
		w_out = output->get_w();

		if(in_x2 - in_x1 > out_x2 - out_x1)
			get_reduce_array(xinput_pixel1, xinput_pixel2, 
				xinput_fraction1, xinput_fraction2, xinput_fraction3, 
				total_x_fraction, 
				in_x1, in_x2, out_x1, out_x2, 
				output->get_w(), in_w);
		else
		{
			get_enlarge_array(xinput_pixel1, xinput_fraction1, 
				xinput_pixel2, xinput_fraction3,
				total_x_fraction, 
				in_x1, in_x2, out_x1, out_x2, 
				output->get_w(), in_w);
		}

		if(in_y2 - in_y1 > out_y2 - out_y1)
 			get_reduce_array(yinput_pixel1, yinput_pixel2, 
				yinput_fraction1, yinput_fraction2, yinput_fraction3, 
				total_y_fraction, 
				in_y1, in_y2, out_y1, out_y2, 
				output->get_h(), in_h);
		else
			get_enlarge_array(yinput_pixel1, yinput_fraction1, 
				yinput_pixel2, yinput_fraction3,
				total_y_fraction, 
				in_y1, in_y2, out_y1, out_y2, 
				output->get_h(), in_h);
			
		if(input_v)
		{
			for(y_out = out_y1_i; y_out < out_y2_i; y_out++)
			{
				for(x_out = out_x1_i; x_out < out_x2_i; x_out++)
				{
					interpolate_pixel_reduce(output_rows[y_out][x_out], input_rows, 
						xinput_pixel1[x_out], xinput_pixel2[x_out], 
						xinput_fraction1[x_out], xinput_fraction2[x_out], xinput_fraction3[x_out], 
						yinput_pixel1[y_out], yinput_pixel2[y_out], 
						yinput_fraction1[y_out], yinput_fraction2[y_out], yinput_fraction3[y_out], 
						a_float);
				}
			}
		}
		else
		if(input_c)
		{
			for(y_out = out_y1_i; y_out < out_y2_i; y_out++)
			{
				for(x_out = out_x1_i; x_out < out_x2_i; x_out++)
				{
					interpolate_pixel_reduce(output_rows[y_out][x_out], input_c, 
						xinput_pixel1[x_out], xinput_pixel2[x_out], 
						xinput_fraction1[x_out], xinput_fraction2[x_out], xinput_fraction3[x_out], 
						yinput_pixel1[y_out], yinput_pixel2[y_out], 
						yinput_fraction1[y_out], yinput_fraction2[y_out], yinput_fraction3[y_out], 
						total_x_fraction[x_out] * total_y_fraction[y_out],
						a_float, in_w, in_h);
				}
			}
		}
		delete [] xinput_pixel1;
		delete [] xinput_pixel2;
		delete [] yinput_pixel1;
		delete [] yinput_pixel2;
		delete [] xinput_fraction1;
		delete [] xinput_fraction2;
		delete [] xinput_fraction3;
		delete [] yinput_fraction1;
		delete [] yinput_fraction2;
		delete [] yinput_fraction3;
	}

	delete [] total_x_fraction;
	delete [] total_y_fraction;
return 0;
}

int OverlayFrame::get_scale_array(int *column_table, int *row_table, 
			int in_x1, int in_y1, int in_x2, int in_y2,
			int out_x1, int out_y1, int out_x2, int out_y2)
{
	int y_out;
	register int i;
	float w_in = in_x2 - in_x1;
	float h_in = in_y2 - in_y1;
	int w_out = out_x2 - out_x1;
	int h_out = out_y2 - out_y1;

	float hscale = w_in / w_out;
	float vscale = h_in / h_out;

	for(i = 0; i < w_out; i++)
	{
		column_table[i] = (int)(hscale * i);
	}

	for(i = 0; i < h_out; i++)
	{
		row_table[i] = (int)(vscale * i) + in_y1;
	}
return 0;
}


int OverlayFrame::get_reduce_array(int *input_pixel1, int *input_pixel2, 
		float *input_fraction1, float *input_fraction2, float *input_fraction3, 
		float *total_fraction, 
		float in_pixel1, float in_pixel2, float out_pixel1, float out_pixel2, 
		int out_total, int in_total)
{
	float out_pixel, out_start, out_end, in_start, in_end;
	int out_pixel_i, out_pixel2_i;
	float scale, total_inputs;

	for(out_pixel_i = 0; out_pixel_i < out_total; out_pixel_i++)
	{
		input_pixel1[out_pixel_i] = 0;
		input_pixel2[out_pixel_i] = 0;
		input_fraction1[out_pixel_i] = 0;
		input_fraction2[out_pixel_i] = 0;
		input_fraction3[out_pixel_i] = 0;
	}

	scale = (in_pixel2 - in_pixel1) / (out_pixel2 - out_pixel1);
	out_pixel2_i = (int)(out_pixel2 + 1);

	for(out_pixel_i = (int)out_pixel1; 
		out_pixel_i < out_pixel2_i; 
		out_pixel_i++)
	{
		out_start = out_pixel_i;
		in_start = (out_start - out_pixel1) * scale + in_pixel1;

		out_end = out_pixel_i + 1;
		in_end = (out_end - out_pixel1) * scale + in_pixel1;

		if(out_start < out_pixel1)
		{
			out_start = out_pixel1;
			in_start = in_pixel1;
		}
		else
		if(out_end > out_pixel2)
		{
			in_end = in_pixel2;
			out_end = out_pixel2;
		}

// Store input fractions
		input_fraction1[out_pixel_i] = (floor(in_start + 1) - in_start) / scale;
		input_fraction2[out_pixel_i] = 1 / scale;
		input_fraction3[out_pixel_i] = (in_end - floor(in_end)) / scale;

		if(in_end >= in_total)
		{ in_end = 0; input_fraction3[out_pixel_i] = 0; }

// Store input pixels
		input_pixel1[out_pixel_i] = (int)in_start;
		input_pixel2[out_pixel_i] = (int)in_end;

// Sanity check.
		if(input_pixel1[out_pixel_i] > input_pixel2[out_pixel_i])
		{
			input_pixel1[out_pixel_i] = input_pixel2[out_pixel_i];
			input_fraction1[out_pixel_i] = 0;
		}

// Get total fraction of output pixel used
		if(input_pixel2[out_pixel_i] > input_pixel1[out_pixel_i])
			total_fraction[out_pixel_i] = input_fraction1[out_pixel_i] + 
				input_fraction2[out_pixel_i] * (input_pixel2[out_pixel_i] - input_pixel1[out_pixel_i] - 1) +
				input_fraction3[out_pixel_i];
		else
			total_fraction[out_pixel_i] = input_fraction1[out_pixel_i] + 
				input_fraction3[out_pixel_i];
	}
return 0;
}

int OverlayFrame::get_enlarge_array(int *input_pixel1, float *pixel1_fraction, 
	int *input_pixel2, float *pixel2_fraction,
	float *total_fraction, 
	float in_pixel1, float in_pixel2, float out_pixel1, float out_pixel2, 
	int out_total, int in_total)
{
	int out_pixel, out_pixel2_i;
	float out_pixels, in_pixels, scale, in_pixel;

	for(out_pixel = 0; out_pixel < out_total; out_pixel++)
	{
		input_pixel1[out_pixel] = 0;
		input_pixel2[out_pixel] = 0;
		pixel1_fraction[out_pixel] = 0;
		pixel2_fraction[out_pixel] = 0;
	}

	out_pixel2_i = (int)(out_pixel2 + 1);
	out_pixels = out_pixel2 - out_pixel1;
	in_pixels = in_pixel2 - in_pixel1;
	scale = in_pixels / out_pixels;

	for(out_pixel = (int)out_pixel1; out_pixel < out_pixel2_i; out_pixel++)
	{
// Projection of output pixel on input pixels
		in_pixel = (out_pixel - out_pixel1) * scale + in_pixel1;
		input_pixel1[out_pixel] = (int)floor(in_pixel);
		input_pixel2[out_pixel] = input_pixel1[out_pixel] + 1;

		if(in_pixel <= in_pixel2) 
			pixel2_fraction[out_pixel] = in_pixel - input_pixel1[out_pixel];
		else 
		{ 
			pixel2_fraction[out_pixel] = 0; 
			input_pixel2[out_pixel] = 0; 
		}

		if(in_pixel >= in_pixel1) 
			pixel1_fraction[out_pixel] = input_pixel2[out_pixel] - in_pixel;
		else 
		{ 
			pixel1_fraction[out_pixel] = 0; 
			input_pixel1[out_pixel] = 0;  
		}

		if(input_pixel2[out_pixel] >= in_total) 
		{ 
			input_pixel2[out_pixel] = 0; 
			pixel2_fraction[out_pixel] = 0; 
		}

		total_fraction[out_pixel] = pixel1_fraction[out_pixel] + pixel2_fraction[out_pixel];
//printf("%.2f ", pixel2_fraction[out_pixel] + pixel1_fraction[out_pixel]);
	}
//printf("\n\n");
return 0;
}



int OverlayFrame::transfer_row_direct(VPixel *output, VPixel *input, int out_columns, 
	int alpha)
{
	if(use_float)
	{
		float a_float;
		a_float = (float)alpha / VMAX;

		for(register int i = 0; i < out_columns; i++)
		{
			pixel_overlay->overlay_pixel_f(output[i], input[i], a_float);
		}
	}
	else
	{
		int a_int = alpha;
#if (VMAX == 65535)
		a_int >>= 8;    // Now 0 - 255
#endif
		for(register int i = 0; i < out_columns; i++)
		{
			pixel_overlay->overlay_pixel_i(output[i], input[i], a_int);
		}
	}
return 0;
}

int OverlayFrame::transfer_row_direct(VPixel *output, unsigned char *input, int out_columns, 
	int alpha)
{
	if(use_float)
	{
		float a_float;
		a_float = (float)alpha / VMAX;

		for(register int i = 0; i < out_columns; i++)
		{
			pixel_overlay->overlay_pixel_f(output[i], &input[i * depth], a_float);
		}
	}
	else
	{
		int a_int = alpha;
#if (VMAX == 65535)
		a_int >>= 8;    // Now 0 - 255
#endif
		for(register int i = 0; i < out_columns; i++)
		{
			pixel_overlay->overlay_pixel_i(output[i], &input[i * depth], a_int);
		}
	}
return 0;
}

int OverlayFrame::transfer_row_scale(VPixel *output, VPixel *input, int out_columns, int *column_table, 
	int alpha)
{
	if(use_float)
	{
		float a_float = (float)alpha / VMAX;

		for(register int i = 0; i < out_columns; i++)
		{
			pixel_overlay->overlay_pixel_f(output[i], input[column_table[i]], a_float);
		}
	}
	else
	{
		int a_int = alpha;
#if (VMAX == 65535)
		a_int >>= 8;     // Now 0 - 255
#endif

		for(register int i = 0; i < out_columns; i++)
		{
			pixel_overlay->overlay_pixel_i(output[i], input[column_table[i]], a_int);
		}
	}
return 0;
}

int OverlayFrame::transfer_row_scale(VPixel *output, unsigned char *input, int out_columns, int *column_table, 
	int alpha)
{
	if(use_float)
	{
		float a_float = (float)alpha / VMAX;

		for(register int i = 0; i < out_columns; i++)
		{
			pixel_overlay->overlay_pixel_f(output[i], &input[column_table[i] * depth], a_float);
		}
	}
	else
	{
		int a_int = alpha;
#if (VMAX == 65535)
		a_int >>= 8;     // Now 0 - 255
#endif

		for(register int i = 0; i < out_columns; i++)
		{
			pixel_overlay->overlay_pixel_i(output[i], &input[column_table[i] * depth], a_int);
		}
	}
return 0;
}



int OverlayFrame::interpolate_pixel_enlarge(VPixel &output, 
	VPixel &input1, VPixel &input2, VPixel &input3, VPixel &input4,
	float &fraction1, float &fraction2, float &fraction3, float &fraction4, 
	float &alpha)
{
	temp_f[0] = .5 + fraction1 * input1.r + fraction2 * input2.r + fraction3 * input3.r + fraction4 * input4.r;
	temp_f[1] = .5 + fraction1 * input1.g + fraction2 * input2.g + fraction3 * input3.g + fraction4 * input4.g;
	temp_f[2] = .5 + fraction1 * input1.b + fraction2 * input2.b + fraction3 * input3.b + fraction4 * input4.b;
	temp_f[3] = .5 + fraction1 * input1.a + fraction2 * input2.a + fraction3 * input3.a + fraction4 * input4.a;
	temp.r = (VWORD)(temp_f[0] > VMAX ? VMAX : temp_f[0]);
	temp.g = (VWORD)(temp_f[1] > VMAX ? VMAX : temp_f[1]);
	temp.b = (VWORD)(temp_f[2] > VMAX ? VMAX : temp_f[2]);
	temp.a = (VWORD)(temp_f[3] > VMAX ? VMAX : temp_f[3]);

	pixel_overlay->overlay_pixel_f(output, temp, alpha);
return 0;
}

int OverlayFrame::interpolate_pixel_enlarge(VPixel &output, 
	unsigned char *input1, unsigned char *input2, unsigned char *input3, unsigned char *input4,
	float &fraction1, float &fraction2, float &fraction3, float &fraction4, 
	float total_fraction, 
	float &alpha, int &in_w, int &in_h)
{
	for(register int j = 0; j < depth; j++)
	{
		temp_f[0] = .5 + fraction1 * *input1++ + fraction2 * *input2++ + fraction3 * *input3++ + fraction4 * *input4++;
		temp_c[j] = (VWORD)(temp_f[0] > VMAX ? VMAX : temp_f[0]);
	}
	pixel_overlay->overlay_unsigned_f(output, temp_c, alpha, (int)(VMAX * total_fraction + 0.5));
return 0;
}

#define VPIXEL_REDUCE_MACRO(i) \
		input_row = &input[i][xinput_pixel1]; \
		input_end = &input[i][xinput_pixel2]; \
 \
/* Compiler error forces this ordering */ \
/* Do first pixel */ \
		temp_f[0] += input_scale1 * input_row->r; \
		temp_f[1] += input_scale1 * input_row->g; \
		temp_f[2] += input_scale1 * input_row->b; \
		temp_f[3] += input_scale1 * input_row->a; \
 \
/* Do last pixel */ \
		if(input_row < input_end) \
		{ \
			temp_f[0] += input_scale3 * input_end->r; \
			temp_f[1] += input_scale3 * input_end->g; \
			temp_f[2] += input_scale3 * input_end->b; \
			temp_f[3] += input_scale3 * input_end->a; \
		} \
 \
/* Do middle pixels */ \
		for(input_row++; input_row < input_end; input_row++) \
		{ \
			temp_f[0] += input_scale2 * input_row->r; \
			temp_f[1] += input_scale2 * input_row->g; \
			temp_f[2] += input_scale2 * input_row->b; \
			temp_f[3] += input_scale2 * input_row->a; \
		}

int OverlayFrame::interpolate_pixel_reduce(VPixel &output, VPixel **input, 
				int &xinput_pixel1, int &xinput_pixel2, 
				float &xinput_fraction1, float &xinput_fraction2, float &xinput_fraction3, 
				int &yinput_pixel1, int &yinput_pixel2, 
				float &yinput_fraction1, float &yinput_fraction2, float &yinput_fraction3, 
				float &a_float)
{
// Try rounding up
	temp_f[0] = temp_f[1] = temp_f[2] = temp_f[3] = .5;

// First row
	input_scale1 = yinput_fraction1 * xinput_fraction1;
	input_scale2 = yinput_fraction1 * xinput_fraction2;
	input_scale3 = yinput_fraction1 * xinput_fraction3;
	VPIXEL_REDUCE_MACRO(yinput_pixel1)

// Last row
	if(yinput_pixel1 < yinput_pixel2)
	{
		input_scale1 = yinput_fraction3 * xinput_fraction1;
		input_scale2 = yinput_fraction3 * xinput_fraction2;
		input_scale3 = yinput_fraction3 * xinput_fraction3;
		VPIXEL_REDUCE_MACRO(yinput_pixel2)

// Middle rows
		if(yinput_pixel2 - yinput_pixel1 > 1)
		{
			input_scale1 = yinput_fraction2 * xinput_fraction1;
			input_scale2 = yinput_fraction2 * xinput_fraction2;
			input_scale3 = yinput_fraction2 * xinput_fraction3;
			for(i = yinput_pixel1 + 1; i < yinput_pixel2; i++)
			{
				VPIXEL_REDUCE_MACRO(i)
			}
		}
	}

	if(temp_f[0] > VMAX) temp_f[0] = VMAX;
	if(temp_f[1] > VMAX) temp_f[1] = VMAX;
	if(temp_f[2] > VMAX) temp_f[2] = VMAX;
	if(temp_f[3] > VMAX) temp_f[3] = VMAX;
	temp.r = (VWORD)(temp_f[0]);
	temp.g = (VWORD)(temp_f[1]);
	temp.b = (VWORD)(temp_f[2]);
	temp.a = (VWORD)(temp_f[3]);

	pixel_overlay->overlay_pixel_f(output, temp, a_float);
return 0;
}

#define UNSIGNED_REDUCE_MACRO(i) \
/* Compiler error forces this ordering */ \
/* Do first pixel */ \
	input_row_c = &input[(i * in_w + xinput_pixel1) * depth]; \
	for(register int j = 0; j < depth; j++) \
		temp_f[j] += (float)(input_row_c[j]) * input_scale1; \
\
/* Do last pixel */ \
	if(xinput_pixel1 < xinput_pixel2) \
	{ \
		input_row_c = &input[(i * in_w + xinput_pixel2) * depth]; \
		for(register int j = 0; j < depth; j++) \
			temp_f[j] += (float)(input_row_c[j]) * input_scale3; \
\
		if(xinput_pixel2 - xinput_pixel1 > 1) \
		{ \
			input_row_c = &input[(i * in_w + xinput_pixel1 + 1) * depth]; \
			input_end_c = &input[(i * in_w + xinput_pixel2 - 1) * depth]; \
			while(input_row_c <= input_end_c) \
			{ \
				for(register int j = 0; j < depth; j++) \
					temp_f[j] += (float)(*input_row_c++) * input_scale2; \
			} \
		} \
	}

int OverlayFrame::interpolate_pixel_reduce(VPixel &output, unsigned char *input, 
				int &xinput_pixel1, int &xinput_pixel2, 
				float &xinput_fraction1, float &xinput_fraction2, float &xinput_fraction3, 
				int &yinput_pixel1, int &yinput_pixel2, 
				float &yinput_fraction1, float &yinput_fraction2, float &yinput_fraction3, 
				float total_fraction, 
				float &a_float, int &in_w, int &in_h)
{
// Try rounding up
	temp_f[0] = temp_f[1] = temp_f[2] = temp_f[3] = .5;

// First row
	input_scale1 = yinput_fraction1 * xinput_fraction1;
	input_scale2 = yinput_fraction1 * xinput_fraction2;
	input_scale3 = yinput_fraction1 * xinput_fraction3;
	UNSIGNED_REDUCE_MACRO(yinput_pixel1)

// Last row
	if(yinput_pixel1 < yinput_pixel2)
	{
		input_scale1 = yinput_fraction3 * xinput_fraction1;
		input_scale2 = yinput_fraction3 * xinput_fraction2;
		input_scale3 = yinput_fraction3 * xinput_fraction3;
		UNSIGNED_REDUCE_MACRO(yinput_pixel2)

// Middle rows
		if(yinput_pixel2 - yinput_pixel1 > 1)
		{
			input_scale1 = yinput_fraction2 * xinput_fraction1;
			input_scale2 = yinput_fraction2 * xinput_fraction2;
			input_scale3 = yinput_fraction2 * xinput_fraction3;
			for(i = yinput_pixel1 + 1; i < yinput_pixel2; i++)
			{
				UNSIGNED_REDUCE_MACRO(i)
			}
		}
	}

	for(register int j = 0; j < depth; j++)
	{
		if(temp_f[j] > 255) temp_f[j] = 255;
		temp_c[j] = (unsigned char)(temp_f[j]);
	}

	pixel_overlay->overlay_unsigned_f(output, temp_c, a_float, (int)(VMAX * total_fraction + 0.5));
return 0;
}






OverlayPixelBase::OverlayPixelBase() { }
OverlayPixelBase::~OverlayPixelBase() { }
int OverlayPixelBase::overlay_pixel_f(VPixel &output, VPixel &input, float &alpha) { return 0;
}
int OverlayPixelBase::overlay_pixel_i(VPixel &output, VPixel &input, int &alpha) { return 0;
}
int OverlayPixelBase::overlay_pixel_f(VPixel &output, unsigned char *input, float &alpha) { return 0;
}
int OverlayPixelBase::overlay_unsigned_f(VPixel &output, unsigned char *input, float &opacity, int alpha) { return 0;
}
int OverlayPixelBase::overlay_pixel_i(VPixel &output, unsigned char *input, int &alpha) { return 0;
}


OverlayPixelNormalAlpha::OverlayPixelNormalAlpha() : OverlayPixelBase() { } 
int OverlayPixelNormalAlpha::overlay_pixel_f(VPixel &output, VPixel &input, float &opacity)
{
	input_opacity_f = (float)input.a * opacity / VMAX;
	output_opacity_f = (1 - input_opacity_f) * output.a / VMAX;

	output.r = (VWORD)(output_opacity_f * output.r + input_opacity_f * input.r);
	output.g = (VWORD)(output_opacity_f * output.g + input_opacity_f * input.g);
	output.b = (VWORD)(output_opacity_f * output.b + input_opacity_f * input.b);
	output.a = input.a > output.a ? input.a : output.a;
return 0;
}
int OverlayPixelNormalAlpha::overlay_pixel_i(VPixel &output, VPixel &input, int &opacity)
{
	input_opacity_i = (opacity * input.a) >> (sizeof(VWORD) * 8);   // now 0-255
	output_opacity_i = ((255 - input_opacity_i) * output.a) >> (sizeof(VWORD) * 8);     // Now 0-255

	output.r = (output_opacity_i * output.r + input_opacity_i * input.r) >> 8;
	output.g = (output_opacity_i * output.g + input_opacity_i * input.g) >> 8;
	output.b = (output_opacity_i * output.b + input_opacity_i * input.b) >> 8;
	output.a = input.a > output.a ? input.a : output.a;
return 0;
}


OverlayPixelNormal::OverlayPixelNormal() : OverlayPixelBase() { } 
int OverlayPixelNormal::overlay_pixel_f(VPixel &output, VPixel &input, float &alpha)
{
	output = input;
return 0;
}
int OverlayPixelNormal::overlay_pixel_i(VPixel &output, VPixel &input, int &alpha)
{
	output = input;
return 0;
}


#define TEST_MAXBOUNDARIES_AND_STORE \
\
	if(r_i > VMAX) r_i = VMAX;\
	if(g_i > VMAX) g_i = VMAX;\
	if(b_i > VMAX) b_i = VMAX;\
\
	output.r = r_i;\
	output.g = g_i;\
	output.b = b_i;


OverlayPixelAdditionAlpha::OverlayPixelAdditionAlpha() : OverlayPixelBase() { } 
int OverlayPixelAdditionAlpha::overlay_pixel_f(VPixel &output, VPixel &input, float &opacity)
{
	input_opacity_f = (float)input.a * opacity / VMAX;
	output_opacity_f = (1 - input_opacity_f) * output.a / VMAX;

	r_i = (long)(input_opacity_f * (input.r + output.r) + output_opacity_f * output.r);
	g_i = (long)(input_opacity_f * (input.g + output.g) + output_opacity_f * output.g);
	b_i = (long)(input_opacity_f * (input.b + output.b) + output_opacity_f * output.b);
	output.a = input.a > output.a ? input.a : output.a;
	TEST_MAXBOUNDARIES_AND_STORE
return 0;
}
int OverlayPixelAdditionAlpha::overlay_pixel_i(VPixel &output, VPixel &input, int &opacity)
{
	input_opacity_i = (opacity * input.a) >> (sizeof(VWORD) * 8);   // now 0-255
	output_opacity_i = ((255 - input_opacity_i) * output.a) >> (sizeof(VWORD) * 8);     // Now 0-255

	r_i = (input_opacity_i * (input.r + output.r) + output_opacity_i * output.r) >> 8;
	g_i = (input_opacity_i * (input.g + output.g) + output_opacity_i * output.g) >> 8;
	b_i = (input_opacity_i * (input.b + output.b) + output_opacity_i * output.b) >> 8;
	output.a = input.a > output.a ? input.a : output.a;
	TEST_MAXBOUNDARIES_AND_STORE
return 0;
}

OverlayPixelAddition::OverlayPixelAddition() : OverlayPixelBase() { } 
int OverlayPixelAddition::overlay_pixel_f(VPixel &output, VPixel &input, float &alpha)
{
	r_i = (long)output.r + input.r;
	g_i = (long)output.g + input.g;
	b_i = (long)output.b + input.b;
	output.a = VMAX;

	TEST_MAXBOUNDARIES_AND_STORE
return 0;
}
int OverlayPixelAddition::overlay_pixel_i(VPixel &output, VPixel &input, int &alpha)
{
	r_i = (long)output.r + input.r;
	g_i = (long)output.g + input.g;
	b_i = (long)output.b + input.b;
	output.a = VMAX;

	TEST_MAXBOUNDARIES_AND_STORE
return 0;
}


#define TEST_MINBOUNDARIES_AND_STORE \
\
	if(r_i < 0) r_i = 0;\
	if(g_i < 0) g_i = 0;\
	if(b_i < 0) b_i = 0;\
\
	output.r = r_i;\
	output.g = g_i;\
	output.b = b_i;

OverlayPixelSubtractAlpha::OverlayPixelSubtractAlpha() : OverlayPixelBase() { } 
int OverlayPixelSubtractAlpha::overlay_pixel_f(VPixel &output, VPixel &input, float &opacity)
{
	input_opacity_f = (float)input.a * opacity / VMAX;
	output_opacity_f = (1 - input_opacity_f) * output.a / VMAX;

	r_i = (long)(input_opacity_f * (input.r - output.r) + output_opacity_f * output.r);
	g_i = (long)(input_opacity_f * (input.g - output.g) + output_opacity_f * output.g);
	b_i = (long)(input_opacity_f * (input.b - output.b) + output_opacity_f * output.b);
	output.a = input.a > output.a ? input.a : output.a;
	TEST_MINBOUNDARIES_AND_STORE
return 0;
}
int OverlayPixelSubtractAlpha::overlay_pixel_i(VPixel &output, VPixel &input, int &opacity)
{
	input_opacity_i = (opacity * input.a) >> (sizeof(VWORD) * 8);   // now 0-255
	output_opacity_i = ((255 - input_opacity_i) * output.a) >> (sizeof(VWORD) * 8);     // Now 0-255

	r_i = (input_opacity_i * (input.r - output.r) + output_opacity_i * output.r) >> 8;
	g_i = (input_opacity_i * (input.g - output.g) + output_opacity_i * output.g) >> 8;
	b_i = (input_opacity_i * (input.b - output.b) + output_opacity_i * output.b) >> 8;
	output.a = input.a > output.a ? input.a : output.a;
	TEST_MINBOUNDARIES_AND_STORE
return 0;
}

OverlayPixelSubtract::OverlayPixelSubtract() : OverlayPixelBase() { } 
int OverlayPixelSubtract::overlay_pixel_f(VPixel &output, VPixel &input, float &alpha)
{
	r_i = (long)input.r - output.r;
	g_i = (long)input.g - output.g;
	b_i = (long)input.b - output.b;
	output.a = VMAX;

	TEST_MINBOUNDARIES_AND_STORE
return 0;
}
int OverlayPixelSubtract::overlay_pixel_i(VPixel &output, VPixel &input, int &alpha)
{
	r_i = (long)input.r - output.r;
	g_i = (long)input.g - output.g;
	b_i = (long)input.b - output.b;
	output.a = VMAX;

	TEST_MINBOUNDARIES_AND_STORE
return 0;
}

OverlayPixelMultiplyAlpha::OverlayPixelMultiplyAlpha() : OverlayPixelBase() { } 
int OverlayPixelMultiplyAlpha::overlay_pixel_f(VPixel &output, VPixel &input, float &opacity)
{
	input_opacity_f = (float)input.a * opacity / VMAX;
	output_opacity_f = (1 - input_opacity_f) * output.a / VMAX;

	r_f = ((float)input.r / VMAX * output.r);
	g_f = ((float)input.g / VMAX * output.g);
	b_f = ((float)input.b / VMAX * output.b);
	output.r = (VWORD)(input_opacity_f * r_f + output_opacity_f * output.r);
	output.g = (VWORD)(input_opacity_f * g_f + output_opacity_f * output.g);
	output.b = (VWORD)(input_opacity_f * b_f + output_opacity_f * output.b);
	output.a = input.a > output.a ? input.a : output.a;
return 0;
}
int OverlayPixelMultiplyAlpha::overlay_pixel_i(VPixel &output, VPixel &input, int &opacity)
{
	input_opacity_i = (opacity * input.a) >> (sizeof(VWORD) * 8);   // now 0-255
	output_opacity_i = ((255 - input_opacity_i) * output.a) >> (sizeof(VWORD) * 8);     // Now 0-255

#if (VMAX == 65535)
	output.r = (input_opacity_i * (((input.r >> 8) * output.r) >> 8) + output_opacity_i * output.r) >> 8;
	output.g = (input_opacity_i * (((input.g >> 8) * output.g) >> 8) + output_opacity_i * output.g) >> 8;
	output.b = (input_opacity_i * (((input.b >> 8) * output.b) >> 8) + output_opacity_i * output.b) >> 8;
#else
	output.r = (input_opacity_i * ((input.r * output.r) >> 8) + output_opacity_i * output.r) >> 8;
	output.g = (input_opacity_i * ((input.g * output.g) >> 8) + output_opacity_i * output.g) >> 8;
	output.b = (input_opacity_i * ((input.b * output.b) >> 8) + output_opacity_i * output.b) >> 8;
#endif

	output.a = input.a > output.a ? input.a : output.a;
return 0;
}

OverlayPixelMultiply::OverlayPixelMultiply() : OverlayPixelBase() { } 
int OverlayPixelMultiply::overlay_pixel_f(VPixel &output, VPixel &input, float &alpha)
{
	output.r = (VWORD)((float)input.r / VMAX * output.r);
	output.g = (VWORD)((float)input.g / VMAX * output.g);
	output.b = (VWORD)((float)input.b / VMAX * output.b);
	output.a = VMAX;
return 0;
}
int OverlayPixelMultiply::overlay_pixel_i(VPixel &output, VPixel &input, int &alpha)
{
#if (VMAX == 65535)
	output.r = (output.r * (input.r >> 8)) >> 8;
	output.g = (output.g * (input.g >> 8)) >> 8;
	output.b = (output.b * (input.b >> 8)) >> 8;
#else
	output.r = (output.r * input.r) >> 8;
	output.g = (output.g * input.g) >> 8;
	output.b = (output.b * input.b) >> 8;
#endif
	output.a = VMAX;
return 0;
}

OverlayPixelDivideAlpha::OverlayPixelDivideAlpha() : OverlayPixelBase() { } 
int OverlayPixelDivideAlpha::overlay_pixel_f(VPixel &output, VPixel &input, float &opacity)
{
	input_opacity_f = (float)input.a * opacity / VMAX;
	output_opacity_f = (1 - input_opacity_f) * output.a / VMAX;

	r_f = (float)input.r / ((float)(output.r + 1) / VMAX);
	g_f = (float)input.g / ((float)(output.g + 1) / VMAX);
	b_f = (float)input.b / ((float)(output.b + 1) / VMAX);
	r_i = (long)(input_opacity_f * r_f + output_opacity_f * output.r);
	g_i = (long)(input_opacity_f * g_f + output_opacity_f * output.g);
	b_i = (long)(input_opacity_f * b_f + output_opacity_f * output.b);
	output.a = input.a > output.a ? input.a : output.a;
	TEST_MAXBOUNDARIES_AND_STORE
return 0;
}
int OverlayPixelDivideAlpha::overlay_pixel_i(VPixel &output, VPixel &input, int &opacity)
{
	input_opacity_i = (opacity * input.a) >> (sizeof(VWORD) * 8);   // now 0-255
	output_opacity_i = ((255 - input_opacity_i) * output.a) >> (sizeof(VWORD) * 8);     // Now 0-255

#if (VMAX == 65535)
	r_i = ((long)input.r << 8) / ((output.r >> 8) + 1);
	g_i = ((long)input.g << 8) / ((output.g >> 8) + 1);
	b_i = ((long)input.b << 8) / ((output.b >> 8) + 1);
#else
	r_i = ((long)input.r << 8) / (output.r + 1);
	g_i = ((long)input.g << 8) / (output.g + 1);
	b_i = ((long)input.b << 8) / (output.b + 1);
#endif

	r_i = (input_opacity_i * r_i + output_opacity_i * output.r) >> 8;
	g_i = (input_opacity_i * g_i + output_opacity_i * output.g) >> 8;
	b_i = (input_opacity_i * b_i + output_opacity_i * output.b) >> 8;
	output.a = input.a > output.a ? input.a : output.a;
	TEST_MAXBOUNDARIES_AND_STORE
return 0;
}

OverlayPixelDivide::OverlayPixelDivide() : OverlayPixelBase() { }
int OverlayPixelDivide::overlay_pixel_f(VPixel &output, VPixel &input, float &alpha)
{
	r_i = (long)((float)input.r / ((float)(output.r + 1) / VMAX));
	g_i = (long)((float)input.g / ((float)(output.g + 1) / VMAX));
	b_i = (long)((float)input.b / ((float)(output.b + 1) / VMAX));
	output.a = VMAX;
	TEST_MAXBOUNDARIES_AND_STORE
return 0;
}
int OverlayPixelDivide::overlay_pixel_i(VPixel &output, VPixel &input, int &alpha)
{
	r_i = (((long)input.r << 8) / (output.r + 1));
	g_i = (((long)input.g << 8) / (output.g + 1));
	b_i = (((long)input.b << 8) / (output.b + 1));
	output.a = VMAX;
	TEST_MAXBOUNDARIES_AND_STORE
return 0;
}

// Various color models for raw input

OverlayPixelAlphaRGB::OverlayPixelAlphaRGB() : OverlayPixelBase() { } 

int OverlayPixelAlphaRGB::overlay_pixel_f(VPixel &output, unsigned char *input, float &opacity)
{
#if (VMAX == 65535)
	r_f = (VWORD)(input[0]) << 8;
	g_f = (VWORD)(input[1]) << 8;
	b_f = (VWORD)(input[2]) << 8;
#else
	r_f = input[0];
	g_f = input[1];
	b_f = input[2];
#endif
	output_opacity_f = (1 - opacity) * output.a / VMAX;

	output.r = (VWORD)(output_opacity_f * output.r + opacity * r_f);
	output.g = (VWORD)(output_opacity_f * output.g + opacity * g_f);
	output.b = (VWORD)(output_opacity_f * output.b + opacity * b_f);
	output.a = VMAX;
return 0;
}

int OverlayPixelAlphaRGB::overlay_unsigned_f(VPixel &output, unsigned char *input, float &opacity, int alpha)
{
#if (VMAX == 65535)
	r_f = (VWORD)(input[0]) << 8;
	g_f = (VWORD)(input[1]) << 8;
	b_f = (VWORD)(input[2]) << 8;
#else
	r_f = input[0];
	g_f = input[1];
	b_f = input[2];
#endif
	output_opacity_f = (1 - opacity) * output.a / VMAX;

	output.r = (VWORD)(output_opacity_f * output.r + opacity * r_f);
	output.g = (VWORD)(output_opacity_f * output.g + opacity * g_f);
	output.b = (VWORD)(output_opacity_f * output.b + opacity * b_f);
	output.a = alpha > output.a ? alpha : output.a;
return 0;
}



int OverlayPixelAlphaRGB::overlay_pixel_i(VPixel &output, unsigned char *input, int &opacity)
{
#if (VMAX == 65535)
	r_i = (VWORD)(input[0]) << 8;
	g_i = (VWORD)(input[1]) << 8;
	b_i = (VWORD)(input[2]) << 8;
#else
	r_i = input[0];
	g_i = input[1];
	b_i = input[2];
#endif
	output_opacity_i = ((255 - opacity) * output.a) >> (sizeof(VWORD) * 8);     // Now 0-255

	output.r = (output_opacity_i * output.r + opacity * r_i) >> 8;
	output.g = (output_opacity_i * output.g + opacity * g_i) >> 8;
	output.b = (output_opacity_i * output.b + opacity * b_i) >> 8;
	output.a = VMAX;
return 0;
}


OverlayPixelRGB::OverlayPixelRGB() : OverlayPixelBase() { } 

int OverlayPixelRGB::overlay_pixel_f(VPixel &output, unsigned char *input, float &alpha)
{
#if (VMAX == 65535)
	output.r = (VWORD)(input[0]) << 8;
	output.g = (VWORD)(input[1]) << 8;
	output.b = (VWORD)(input[2]) << 8;
#else
	output.r = input[0];
	output.g = input[1];
	output.b = input[2];
#endif
	output.a = VMAX;
return 0;
}

int OverlayPixelRGB::overlay_unsigned_f(VPixel &output, unsigned char *input, float &opacity, int alpha)
{
	overlay_pixel_f(output, input, opacity);
return 0;
}

int OverlayPixelRGB::overlay_pixel_i(VPixel &output, unsigned char *input, int &alpha)
{
#if (VMAX == 65535)
	output.r = (VWORD)(input[0]) << 8;
	output.g = (VWORD)(input[1]) << 8;
	output.b = (VWORD)(input[2]) << 8;
#else
	output.r = input[0];
	output.g = input[1];
	output.b = input[2];
#endif
	output.a = alpha;
return 0;
}

OverlayPixelAlphaRGBA::OverlayPixelAlphaRGBA() : OverlayPixelBase() { } 

int OverlayPixelAlphaRGBA::overlay_pixel_f(VPixel &output, unsigned char *input, float &opacity)
{
#if (VMAX == 65535)
	r_f = (VWORD)(input[0]) << 8;
	g_f = (VWORD)(input[1]) << 8;
	b_f = (VWORD)(input[2]) << 8;
	a_f = (VWORD)(input[3]) << 8;
#else
	r_f = input[0];
	g_f = input[1];
	b_f = input[2];
	a_f = input[3];
#endif
	input_opacity_f = a_f * opacity / VMAX;
	output_opacity_f = (1 - input_opacity_f) * output.a / VMAX;

	output.r = (VWORD)(output_opacity_f * output.r + input_opacity_f * r_f);
	output.g = (VWORD)(output_opacity_f * output.g + input_opacity_f * g_f);
	output.b = (VWORD)(output_opacity_f * output.b + input_opacity_f * b_f);
	output.a = (VWORD)(a_f > output.a ? a_f : output.a);
return 0;
}

int OverlayPixelAlphaRGBA::overlay_unsigned_f(VPixel &output, unsigned char *input, float &opacity, int alpha)
{
	overlay_pixel_f(output, input, opacity);
return 0;
}

int OverlayPixelAlphaRGBA::overlay_pixel_i(VPixel &output, unsigned char *input, int &opacity)
{
#if (VMAX == 65535)
	r_i = (int)(input[0]) << 8;
	g_i = (int)(input[1]) << 8;
	b_i = (int)(input[2]) << 8;
	a_i = (int)(input[3]) << 8;
#else
	r_i = input[0];
	g_i = input[1];
	b_i = input[2];
	a_i = input[3];
#endif
	input_opacity_i = (opacity * a_i) >> (sizeof(VWORD) * 8);   // now 0-255
	output_opacity_i = ((255 - input_opacity_i) * output.a) >> (sizeof(VWORD) * 8);     // Now 0-255

	output.r = (output_opacity_i * output.r + input_opacity_i * r_i) >> 8;
	output.g = (output_opacity_i * output.g + input_opacity_i * g_i) >> 8;
	output.b = (output_opacity_i * output.b + input_opacity_i * b_i) >> 8;
	output.a = a_i > output.a ? a_i : output.a;
return 0;
}


OverlayPixelRGBA::OverlayPixelRGBA() : OverlayPixelBase() { } 

int OverlayPixelRGBA::overlay_pixel_f(VPixel &output, unsigned char *input, float &alpha)
{
#if (VMAX == 65535)
	output.r = (VWORD)(input[0]) << 8;
	output.g = (VWORD)(input[1]) << 8;
	output.b = (VWORD)(input[2]) << 8;
	output.a = (VWORD)(input[3]) << 8;
#else
	output.r = input[0];
	output.g = input[1];
	output.b = input[2];
	output.a = input[3];
#endif
return 0;
}

int OverlayPixelRGBA::overlay_unsigned_f(VPixel &output, unsigned char *input, float &opacity, int alpha)
{
	overlay_pixel_f(output, input, opacity);
return 0;
}

int OverlayPixelRGBA::overlay_pixel_i(VPixel &output, unsigned char *input, int &alpha)
{
#if (VMAX == 65535)
	output.r = (VWORD)(input[0]) << 8;
	output.g = (VWORD)(input[1]) << 8;
	output.b = (VWORD)(input[2]) << 8;
	output.a = (VWORD)(input[3]) << 8;
#else
	output.r = input[0];
	output.g = input[1];
	output.b = input[2];
	output.a = input[3];
#endif
return 0;
}
