#include <string.h>
#include "aedit.h"
#include "aedits.h"
#include "assets.h"
#include "atrack.h"
#include "autoconf.h"
#include "edit.h"
#include "cache.h"
#include "datatype.h"
#include "file.h"
#include "filehtal.h"
#include "floatautos.h"
#include "mainwindow.h"
#include "pluginbuffer.h"
#include "trackcanvas.h"
#include "tracks.h"

ATrack::ATrack(MainWindow *mwindow, Tracks *tracks) : Track(mwindow, tracks)
{
	data_type = TRACK_AUDIO;
}

ATrack::~ATrack()
{
	delete edits;
	for(int i = 0; i < mwindow->output_channels; i++) delete pan_autos[i];
}

int ATrack::create_derived_objs(int flash)
{
//printf("ATrack::create_derived_objects 1\n");
	int i;
	edits = new AEdits(mwindow, this);

	for(i = 0; i < mwindow->output_channels; i++)
	{
		pan_autos[i] = new FloatAutos(this, MEPURPLE, -1, 1, PAN_AUTO_H, 1);
		pan_autos[i]->create_objects();
	}
	
	fade_autos = new FloatAutos(this, LTGREY, -46, 46);
	fade_autos->create_objects();

	if(mwindow->gui) 
	{
		if(mwindow->tracks_vertical)
		draw(pixel, mwindow->zoom_track, 0, tracks->canvas->get_h(), flash);
		else
		draw(0, tracks->canvas->get_w(), pixel, mwindow->zoom_track, flash);
	}
return 0;
}

int ATrack::set_index_files(int flash, Asset *asset)
{
	int result = 0;
	AEdit* current;

	if(mwindow->gui)
	{
		result = 1;
		for(current = (AEdit*)edits->first; current; current = (AEdit*)NEXT)
		{
// check for matching asset
			if(current->asset && current->asset == asset)
			{
				if(mwindow->tracks_vertical)
					current->set_index_file(flash, pixel + mwindow->zoom_track / 2, 0, 0, tracks->canvas->get_w(), tracks->canvas->get_h());
				else
					current->set_index_file(flash, pixel + mwindow->zoom_track / 2, 0, 0, tracks->canvas->get_w(), tracks->canvas->get_h());

				result = 0;
			}
		}
	}

	return result;
return 0;
}


int ATrack::save_derived(FileHTAL *htal)
{
	int i;
	edits->save(htal);

	
	for(i = 0; i < mwindow->output_channels; i++)
	{
		if(pan_autos[i]->total())
		{
			htal->tag.set_title("PANAUTOS");
			htal->tag.set_property("CHANNEL", i);
			htal->append_tag();
			pan_autos[i]->save(htal);
			htal->tag.set_title("/PANAUTOS");
			htal->append_tag();
			htal->append_newline();
		}
	}
return 0;
}

int ATrack::load_derived(FileHTAL *htal, int automation_only, int edits_only, int load_all, int &current_channel)
{
	if(htal->tag.title_is("PANAUTOS"))
	{
		current_channel = htal->tag.get_property("CHANNEL", current_channel);
		if(current_channel < mwindow->output_channels) pan_autos[current_channel++]->load(htal, "/PANAUTOS");
	}
return 0;
}

int ATrack::change_channels(int oldchannels, int newchannels)
{
	int i;
	
	if(oldchannels > newchannels)
// delete extra channels
	{
		for(i = newchannels; i < oldchannels; i++)
		{
			delete(pan_autos[i]);
		}
	}
	else
// add new channels
	{
		for(i = oldchannels; i < newchannels; i++)
		{
			pan_autos[i] = new FloatAutos(this, MEPURPLE, -1, 1, PAN_AUTO_H, 1);
			pan_autos[i]->create_objects();
		}
	}
return 0;
}

int ATrack::render(PluginBuffer *shared_output, 
			   long offset, 
			   long input_len, 
               long input_position) // always start of range
{
	Edit *current_edit;

	bzero((char*)shared_output->get_data() + offset * sizeof(float), input_len * sizeof(float));

	for(current_edit = edits->first; 
		current_edit;
		current_edit = current_edit->next)
	{
		long edit_start = current_edit->startproject;
		long edit_end = current_edit->startproject + current_edit->length + current_edit->feather_right;

		if((edit_start >= input_position && edit_start < input_position + input_len)
			 ||
			(edit_end > input_position && edit_end <= input_position + input_len)
			 ||
			(edit_start <= input_position && edit_end >= input_position + input_len))
		{
			((AEdit*)current_edit)->render(shared_output, offset, input_len, input_position);
		}
	}

	return 0;
return 0;
}

// =========================== drawing commands ==================

int ATrack::draw_derived(int x, int w, int y, int h, int flash)
{       // make sure this track is visible
	int center_pixel;    // pixel of center line in canvas
	center_pixel = pixel + mwindow->zoom_track / 2;

	tracks->canvas->set_color(RED);        // draw zero line
	if(mwindow->tracks_vertical)
	tracks->canvas->draw_line(center_pixel, y, center_pixel, y + h);
	else
	tracks->canvas->draw_line(x, center_pixel, x + w, center_pixel);
return 0;
}


int ATrack::draw_autos_derived(float view_start, float zoom_units, AutoConf *auto_conf)
{
	int i;
	
	for(i = 0; i < mwindow->output_channels; i++)
	{
		if(auto_conf->pan[i]) 
			pan_autos[i]->draw(tracks->canvas, 
							pixel, 
							mwindow->zoom_track, 
							zoom_units, 
							view_start, 
							mwindow->tracks_vertical);
	}
return 0;
}

int ATrack::draw_floating_autos_derived(float view_start, float zoom_units, AutoConf *auto_conf, int flash)
{
	int i;

	for(i = 0; i < mwindow->output_channels; i++)
	{
		if(auto_conf->pan[i]) 
			pan_autos[i]->draw_floating_autos(tracks->canvas, 
									pixel, 
									mwindow->zoom_track, 
									zoom_units, 
									view_start, 
									mwindow->tracks_vertical, flash);
	}
return 0;
}

long ATrack::length()
{
	return edits->end();
}

int ATrack::get_dimensions(float &view_start, float &view_units, float &zoom_units)
{
	view_start = (float)mwindow->view_start;
	view_units = (float)tracks->view_samples();
	zoom_units = (float)mwindow->zoom_sample;
return 0;
}

long ATrack::samples_to_units(long &samples)
{
	return samples;
}


long ATrack::units_to_samples(long &units)
{
	return units;
}

int ATrack::copy_derived(long start, long end, FileHTAL *htal)
{
// automation is taken care of by Track::copy_automation
return 0;
}

int ATrack::copy_automation_derived(AutoConf *auto_conf, long selectionstart, long selectionend, FileHTAL *htal)
{
	int i, result;

	for(i = 0, result = 0; i < mwindow->output_channels; i++)
	{
		if(auto_conf->pan[i])
		{
			htal->tag.set_title("PANAUTOS");
			htal->tag.set_property("CHANNEL", i);
			htal->append_tag();

			pan_autos[i]->copy(selectionstart, selectionend, htal, 1);

			htal->tag.set_title("/PANAUTOS");
			htal->append_tag();
			htal->append_newline();
			result = 1;
		}
	}

	return result;
return 0;
}


int ATrack::paste_automation_derived(long selectionstart, long selectionend, long total_length, FileHTAL *htal, int shift_autos, int &current_pan)
{
// only used for automation editing routines
	if(htal->tag.title_is("PANAUTOS") && current_pan < MAXCHANNELS)
	{
		current_pan = htal->tag.get_property("CHANNEL", current_pan);
		pan_autos[current_pan++]->paste(selectionstart, selectionend, total_length, htal, "/PANAUTOS", 1, shift_autos);
	}
	return 1;
return 0;
}

int ATrack::clear_automation_derived(AutoConf *auto_conf, long selectionstart, long selectionend, int shift_autos)
{
// used when clearing just automation
	int i, result;

	for(i = 0, result = 0; i < mwindow->output_channels && !result; i++)
		if(auto_conf->pan[i]) result = 1;

	for(i = 0; i < mwindow->output_channels && result; i++)
	{
		pan_autos[i]->clear(selectionstart, selectionend, 1, shift_autos);
	}
	
	return result;
return 0;
}

int ATrack::paste_derived(long start, long end, long total_length, FileHTAL *htal, int &current_channel)
{
	if(!strcmp(htal->tag.get_title(), "PANAUTOS"))
	{
		current_channel = htal->tag.get_property("CHANNEL", current_channel);

		if(current_channel < mwindow->output_channels) 
			pan_autos[current_channel++]->paste(start, end, total_length, htal, "/PANAUTOS", mwindow->autos_follow_edits);
		return 1;
	}
	return 0;
return 0;
}

int ATrack::paste_output(long startproject, long endproject, long startsource, long endsource, int channel, Asset *asset)
{
	int result = 0;

	result = ((AEdits*)edits)->paste_edit(startproject, endproject, startsource, endsource - startsource, channel, asset);
	//if(!result && mwindow->autos_follow_edits)
	//{
	//	paste_auto_silence(startproject, endproject);
	//}
	return result;
return 0;
}

int ATrack::clear_derived(long start, long end)
{
// used when clearing everything
	int i;
	if(mwindow->autos_follow_edits)
	{
		for(i = 0; i < mwindow->output_channels; i++)
		{
			pan_autos[i]->clear(start, end, 1, 1);
		}
	}
return 0;
}

int ATrack::paste_auto_silence_derived(long start, long end)
{
	for(int i = 0; i < mwindow->output_channels; i++)
	{
		pan_autos[i]->paste_silence(start, end);
	}
return 0;
}

int ATrack::select_auto_derived(float zoom_units, float view_start, AutoConf *auto_conf, int cursor_x, int cursor_y)
{
	int result = 0;
	int i, temp;

	for(i = mwindow->output_channels - 1, result = 0; i >= 0 && !result; i--)
	{
		if(auto_conf->pan[i]) 
			result = pan_autos[i]->select_auto(tracks->canvas, 
											pixel, 
											mwindow->zoom_track, 
											zoom_units, 
											view_start, 
											cursor_x, 
											cursor_y, 
											mwindow->tracks_vertical);
	}
	
	return result;
return 0;
}

int ATrack::move_auto_derived(float zoom_units, float view_start, AutoConf *auto_conf, int cursor_x, int cursor_y, int shift_down)
{
	int result, i;
	result = 0;
	
	for(i = 0; i < mwindow->output_channels && !result; i++)
	{
		if(auto_conf->pan[i]) 
			result = pan_autos[i]->move_auto(tracks->canvas, 
										pixel, 
										mwindow->zoom_track, 
										zoom_units, 
										view_start, 
										cursor_x, 
										cursor_y, 
										shift_down, 
										mwindow->tracks_vertical);
	}
		
	return result;
return 0;
}

int ATrack::release_auto_derived()
{
	int result, i;
	result = 0;

	for(i = 0; i < mwindow->output_channels && !result; i++)
	{
		result = pan_autos[i]->release_auto();
	}
		
	return result;
return 0;
}

int ATrack::scale_time_derived(float rate_scale, int scale_edits, int scale_autos, long start, long end)
{
	for(int i = 0; i < mwindow->output_channels; i++)
	{
		pan_autos[i]->scale_time(rate_scale, scale_edits, scale_autos, start, end);
	}
return 0;
}

