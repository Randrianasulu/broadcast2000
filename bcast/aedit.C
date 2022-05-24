#include <string.h>
#include "assets.h"
#include "edits.h"
#include "aedit.h"
#include "cache.h"
#include "file.h"
#include "filehtal.h"
#include "indexfile.h"
#include "mainwindow.h"
#include "patch.h"
#include "pluginbuffer.h"
#include "trackcanvas.h"
#include "tracks.h"


AEdit::AEdit(MainWindow *mwindow, AEdits *edits)
 : Edit(mwindow, (Edits*)edits)
{
	this->aedits = edits;
	channel = 0;
}

AEdit::~AEdit() { }

int AEdit::set_index_file(int flash, int center_pixel, int x, int y, int w, int h)
{
	if(asset && asset->index_zoom <= mwindow->zoom_sample)
		draw(flash, center_pixel, x, w, y, h, 1);
return 0;
}

int AEdit::load_properties_derived(FileHTAL *htal)
{
	length = htal->tag.get_property("SAMPLES", (long)0);
	channel = htal->tag.get_property("CHANNEL", (long)0);
	return 0;
return 0;
}

// ============================= edit drawing commands ===========================

int AEdit::draw(int flash, 
			int center_pixel, 
			int x, 
			int w, 
			int y, 
			int h, 
			int set_index_file)
{
	if(transition)
	{
// Edit is transition
		draw_transition(flash, center_pixel, x, w, y, h, set_index_file);
	}

	if(!asset) return 0;
	if(asset->silence) return 0;
	if(!edits->track->get_patch_of()->draw) return 0;
	if(channel >= asset->channels) return 0;

// variables
	long view_start = mwindow->view_start;
	int vertical = mwindow->tracks_vertical;
	long zoom_sample = mwindow->zoom_sample;

	view_start += vertical ? (long)y * zoom_sample : (long)x * zoom_sample;    
	long view_end = view_start + (vertical ? ((long)h * zoom_sample) : ((long)w * zoom_sample));
	long endproject = startproject + length;
	long startsource_edit;             // start of source in edit
	long endsource_edit;               // end of source in edit
	int pixel_screen;         // pixel of line to be drawn on screen
	int result;

	if((startproject >= view_start && startproject <= view_end) ||  // startproject in range
		 (endproject <= view_end && endproject >= view_start) ||	   // endproject in range
		 (startproject <= view_start && endproject >= view_end))    // range in project
	{
// edit is visible
// set additional variables
		startsource_edit = startsource; // start of source to be drawn
		endsource_edit = startsource + length; // end of source to be drawn

		if(startproject < view_start)
		{         // start is after start of edit in project
			startsource_edit += view_start - startproject;
		}
		if(endproject > view_end)
		{         // end is before end of edit in project
			endsource_edit -= endproject - view_end;
		}

// line pixel on screen up with startproject
		pixel_screen = vertical ? y : x;
		if(startproject > view_start) pixel_screen += (startproject - view_start) / zoom_sample;

// only draw the last bit for index building
		if(set_index_file && !transition)
		{
			if(asset->old_index_end > startsource_edit)
			{
				pixel_screen += (asset->old_index_end - startsource_edit) / zoom_sample;
				startsource_edit = asset->old_index_end;
			}
		}

// blend in with neighboring segments
		if(startsource_edit >= startsource + zoom_sample){ startsource_edit -= zoom_sample; pixel_screen -= 1; }
		if(endsource_edit <= startsource + length - zoom_sample){ endsource_edit += zoom_sample; }

		if(!asset->silence)
		{
// edit isn't silence or transition so draw a waveform
// attempt use index file
			result = 0;         // 0 use index         1 don't use index
			if(zoom_sample > 1)
			{
				if(asset->index_status != 3)
				{
					if(asset->index_status == 1)
					{
// try to get the index zoom now since the asset doesn't already have it
						IndexFile indexfile(mwindow);
						if(indexfile.open_index(mwindow, asset))
						{
// failed
							result = 1;
						}
						else
						{
							indexfile.close_index();
						}
					}

					if(!result && asset->index_zoom > zoom_sample)
					{
// don't use the index file
						result = 1;
					}

					if(!result)
					{
						IndexFile indexfile(mwindow);

// open the index file
						if(indexfile.open_index(mwindow, asset))
						{  
// failed
							result = 1;
						}
						else
						{
// get the range to draw when being built
							if(asset->index_status == 2)
							{
								if(asset->index_end < endsource_edit) 
								{
									endsource_edit = asset->index_end;
								}
							}

// ==================================== draw the index
							if(endsource_edit > startsource_edit)
							{
								indexfile.draw_index(mwindow->tracks->canvas, 
												pixel_screen, 
												center_pixel, 
												mwindow->zoom_track, 
												zoom_sample, 
												mwindow->zoom_y / 2, 
												startsource_edit, 
												endsource_edit, 
												channel, 
												vertical);
							}

							indexfile.close_index();
							result = 0;  // success
						}
					}
				}

// draw directly
// index wasn't available or zoom was too small
				if(result || asset->index_status == 3)
				{
					if(asset->index_status == 3 || asset->index_zoom > zoom_sample)
					{
						draw_source(pixel_screen, center_pixel, mwindow->zoom_track, zoom_sample, mwindow->zoom_y / 2, startsource_edit, endsource_edit, vertical);
						result = 0;
					}
				}
			}
			else
			{         // single pixel resolution
// single pixel resolution used	
				draw_direct(pixel_screen, center_pixel, mwindow->zoom_track, zoom_sample, mwindow->zoom_y / 2, startsource_edit, endsource_edit, vertical);
			}
		}
		
// flash just this edit
		if(flash) 
		{
			if(vertical) 
			mwindow->tracks->canvas->flash(center_pixel - mwindow->zoom_track / 2, pixel_screen, mwindow->zoom_track, (endsource_edit - startsource_edit) / zoom_sample + 1);
			else
			mwindow->tracks->canvas->flash(pixel_screen, center_pixel - mwindow->zoom_track / 2, (endsource_edit - startsource_edit) / zoom_sample + 1, mwindow->zoom_track);
		}
	}
return 0;
}

int AEdit::draw_source(int pixel, int center_pixel, int h, long zoom_sample, long zoomy, long startsource, long endsource, int vertical)
{
	File *source;
	int result;
	long buffersize = 64000;      // amount to read at a time
	long fragment_size = buffersize;
	float *buffer;
	PluginBuffer *file_buffer;

	if(!(source = mwindow->cache->check_out(asset)))
	{
// couldn't open source file
		printf("AEdit::draw_source Couldn't open %s.\n", asset->path);
		return 1;
	}

	long length = endsource - startsource; // samples in segment to draw
	file_buffer = new PluginBuffer(buffersize + 1, sizeof(float));
	buffer = (float*)file_buffer->get_data();
	source->set_position(startsource);
	source->set_channel(channel);
	long position;      // current position in source
	float highsample;
	float lowsample;    // first sample
	long zoom_sampleframe = 0;
	mwindow->tracks->canvas->set_color(GREEN);

	for(position = 0; position < length; position += fragment_size)
	{
		if(length - position < fragment_size && fragment_size == buffersize)
			fragment_size = length - position + 1;

		if(source->read_samples(file_buffer, 0, fragment_size))
		{
// read failed
			mwindow->cache->check_in(asset);       // source
			delete file_buffer;
			return 1;
		}
		
		long bufferposition = 0;
		if(position == 0)
		{
			highsample = buffer[bufferposition];
			lowsample = buffer[bufferposition];
			bufferposition++;
		}
		
		while(bufferposition < fragment_size)
		{
			if(zoom_sampleframe == zoom_sample)
			{    // draw column and reset
				if(vertical)
				mwindow->tracks->canvas->draw_line((int)(center_pixel + highsample * zoomy), pixel, (int)(center_pixel + lowsample * zoomy), pixel);
				else
				mwindow->tracks->canvas->draw_line(pixel, (int)(center_pixel - highsample * zoomy), pixel, (int)(center_pixel - lowsample * zoomy));

				zoom_sampleframe = 0;
				pixel++;
			}
			
			if(zoom_sampleframe > 0)
			{     // lowsample and highsample are set
				if(buffer[bufferposition] < lowsample) lowsample = buffer[bufferposition];
				else if(buffer[bufferposition] > highsample) highsample = buffer[bufferposition];
			}
			else
			{     // first zoom_sampleframe should initialize lowsample and highsample
				if(buffer[bufferposition] < lowsample)
				{        // all samples are below lowsample
					highsample = lowsample;
				}
				else if(buffer[bufferposition] > highsample) 
				{        // all samples are above highsample
					lowsample = highsample;
				}
				else
				{        // current sample is in between lowsample and highsample
					lowsample = highsample = buffer[bufferposition];
				}
			}
			bufferposition++;
			zoom_sampleframe++;
		}
	}
// draw last column
//canvas->draw_line(x, (int)(center_pixel - highsample * zoomy), x, (int)(center_pixel - lowsample * zoomy));

	mwindow->cache->check_in(asset);
	delete file_buffer;
return 0;
}

int AEdit::draw_direct(int pixel, int center_pixel, int h, long zoom_sample, long zoomy, long startsource, long endsource, int vertical)
{
	File *source;
	PluginBuffer *file_buffer;
	
	if(!(source = mwindow->cache->check_out(asset)))
	{
// couldn't open source file
		printf("Couldn't open %s.\n", asset->path);
		return 1;
	}

	long length = endsource - startsource; // samples in segment to draw
	file_buffer = new PluginBuffer(length + 1, sizeof(float));
	float *buffer;
	buffer = (float*)file_buffer->get_data();

	source->set_position(startsource);
	source->set_channel(channel);
	if(source->read_samples(file_buffer, 0, length + 1))
	{                    // read failed
		mwindow->cache->check_in(asset);       // source
		delete buffer;             // read buffer
		return 1;
	}
	
	mwindow->tracks->canvas->set_color(GREEN);

	float oldsample, newsample;
	long position = 0;
	
	newsample = buffer[position++];
	pixel++;
	
	for(;position < length; position++, pixel++)
	{
		oldsample = newsample;
		newsample = buffer[position];
		if(vertical)
		mwindow->tracks->canvas->draw_line((int)(center_pixel + oldsample * zoomy), pixel - 1, (int)(center_pixel + newsample * zoomy), pixel);
		else
		mwindow->tracks->canvas->draw_line(pixel - 1, (int)(center_pixel - oldsample * zoomy), pixel, (int)(center_pixel - newsample * zoomy));
	}
	
	mwindow->cache->check_in(asset);
	delete file_buffer;
return 0;
}

// ========================================== editing

int AEdit::copy_properties_derived(FileHTAL *htal, long length_in_selection)
{
	htal->tag.set_property("SAMPLES", length_in_selection);
	htal->tag.set_property("CHANNEL", (long)channel);
return 0;
}

int AEdit::get_handle_parameters(long &left, long &right, long &left_sample, long &right_sample, float view_start, float zoom_units)
{
	left = (long)((startproject - view_start) / zoom_units);
	right = (long)(((startproject + length) - view_start) / zoom_units);
	left_sample = startproject;
	right_sample = startproject + length;
return 0;
}


int AEdit::dump_derived()
{
	//printf("	channel %d\n", channel);
return 0;
}


long AEdit::get_source_end()
{
	if(!asset || asset->silence) return -1;   // Infinity

	File source;
	if(source.try_to_open_file(mwindow->plugindb, asset, 1, 0))
	{
// couldn't open source file
		printf("Edits::modify_handles Couldn't open %s.\n", asset->path);
		return -1;
	}

	long endsource = source.get_audio_length();
	source.close_file();
	return endsource;
}


int AEdit::render(PluginBuffer *shared_output, 
			   long offset, 
			   long input_len, 
               long input_position)
{
	long input_start = input_position;		   // start of render in project
	long output_start = 0;  	// start position in output buffer of current edit
	long input_end = input_position + input_len; 			// end of render in project

// test for asset
	if(!asset || asset->silence)
	{   // skip the edit
		return 0;
	}
	else
// edit contains data
	{
		File *source;
		if(!(source = mwindow->cache->check_out(asset)))
		{
// couldn't open source file / skip the edit
			printf("AEdit::render Couldn't open %s.\n", asset->path);
			return 1;
		}
		else
		{
// render the edit
// add the feather range
			long lfeather_start=0, lfeather_end=0, lfeather_len=0;
			float lfeather_slope=0, lfeather_gain=0;
			long rfeather_start=0, rfeather_end=0, rfeather_len=0;

// get the left feather range and slope
			lfeather_len = feather_left;
			lfeather_start = startproject;
			lfeather_end = startproject + lfeather_len;
			if(lfeather_len)
			{
				lfeather_slope = (float)1 / lfeather_len;
				lfeather_gain = 0;
			}

// get the right feather range and slope
			rfeather_len = feather_right;
			rfeather_start = startproject + length;
			rfeather_end = rfeather_start + rfeather_len;


// Trim ranges
			if(lfeather_end < input_position)
			{
				lfeather_start = lfeather_end = input_position;
				lfeather_len = 0;
			}
			else
			if(lfeather_start < input_position)
			{
				lfeather_gain = lfeather_slope * (input_position - lfeather_start);
				lfeather_start = input_position;
				lfeather_len = lfeather_end - lfeather_start;
			}

			if(rfeather_start > input_end)
			{
				rfeather_start = rfeather_end = input_end;
				rfeather_len = 0;
			}
			else
			if(rfeather_end > input_end)
			{
				rfeather_end = input_end;
				rfeather_len = rfeather_end - rfeather_start;
			}

// get source file positions
			long startsource, lengthsource;
			startsource = this->startsource + lfeather_start - startproject;
			lengthsource = rfeather_start + rfeather_len - lfeather_start;

// source ends after end of file
			if(startsource + lengthsource > source->get_audio_length())
			{
				rfeather_len -= startsource + lengthsource - source->get_audio_length();
				if(rfeather_len < 0) rfeather_len = 0;
				lengthsource -= startsource + lengthsource - source->get_audio_length();
			}

// set the start position of the edit in the output buffer
			output_start = lfeather_start - input_start;

// read the data
			source->set_position(startsource);
			source->set_channel(channel);

			source->render_samples(shared_output, 
				output_start + offset, 
				lengthsource, 
				lfeather_len, lfeather_gain, lfeather_slope);

			mwindow->cache->check_in(asset);
		}
	}
	return 0;
return 0;
}
