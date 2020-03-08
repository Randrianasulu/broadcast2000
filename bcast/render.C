#include <string.h>
#include "arender.h"
#include "assets.h"
#include "compresspopup.h"
#include "defaults.h"
#include "edits.h"
#include "errorbox.h"
#include "file.h"
#include "filesystem.h"
#include "formatcheck.h"
#include "formatpopup.h"
#include "formattools.h"
#include "labels.h"
#include "mainundo.h"
#include "mainwindow.h"
#include "module.h"
#include "neworappend.h"
#include "patchbay.h"
#include "playabletracks.h"
#include "playbackengine.h"
#include "preferences.h"
#include "progressbox.h"
#include "timebar.h"
#include "tracks.h"
#include "quicktime.h"
#include "vrender.h"
#include "render.h"
#include "renderengine.h"
#include "vedit.h"
#include "vframe.h"
#include "videoconfig.h"
#include "videodevice.h"

#include <ctype.h>

Render::Render(Defaults *defaults, MainWindow *mwindow, int list_mode)
 : BC_MenuItem((char*)(list_mode ? "Render list..." : "Render...")), Thread()
{
	this->defaults = defaults;
	this->mwindow = mwindow;
	this->list_mode = list_mode;
	in_progress = 0;
}

Render::~Render()
{
}

int Render::handle_event() { start(); return 0;
}

int Render::check_asset(Asset &asset)
{
	if(do_video && mwindow->patches->total_playable_vtracks() &&
		(asset.format == MOV || asset.format == JPEG_LIST || asset.format == PNG || asset.format == JPEG || asset.format == FILE_TIFF))
	{
		asset.video_data = 1;
		asset.layers = 1;
		asset.width = mwindow->output_w;
		asset.height = mwindow->output_h;
	}
	else
	{
		do_video = 0;
		asset.layers = 0;
	}

	if(do_audio && mwindow->patches->total_playable_atracks())
	{
		asset.audio_data = 1;
		asset.channels = mwindow->output_channels;
		if(asset.format == MOV) asset.byte_order = 0;
	}
	else
	{
		do_audio = 0;
		asset.channels = 0;
	}
//printf("Render::check_asset 3\n");
return 0;
}

void Render::run()
{
	if(in_progress) 
		return;
	else
		in_progress = 1;

	int result, format_error;
	Asset asset("");
	char string[1024];
// Selection to render in samples
	long start, end;
// Total selection to render
	long total_start, total_end;
// Total length in samples
	long total_length;
	int last_buffer;
	Label *current_label;
	VideoDevice *video_device;

// Get total range to render
	load_defaults(&asset);
	mwindow->get_affected_range(&start, &end);
	total_start = start;
	total_end = end;
	asset.frame_rate = mwindow->frame_rate;
	asset.rate = mwindow->sample_rate;
	total_length = end - start;
	total_files = 0;

	for(current_label = mwindow->timebar->labels->first;
		current_label; current_label = current_label->next)
	{
		if(current_label->position > total_start && current_label->position <= total_end)
			total_files++;
	}
	if(total_files == 0) total_files = 1;

	if(total_length == 0)
	{
		in_progress = 0;
		return;
	}

	check_asset(asset);

	do
	{
		{
			RenderWindow window(this, &asset);
			window.create_objects();
			result = window.run_window();
			asset.video_data = do_video;
			asset.audio_data = do_audio;
		}

		if(!result)
		{
// Check the asset format for errors.
			FormatCheck format_check(&asset);
			format_error = format_check.check_format();
// Check the filename for a number.
			if(!format_error) format_error = check_numbering(asset);
		}
	}while(format_error && !result);

	check_asset(asset);
	save_defaults(&asset);

	if(!result)
	{
		FileSystem fs;
		fs.complete_path(asset.path);
		File file;
		long startsource_sample;
		long startsource_frame;
		int file_number = 1;
		int done = 0;

// Thread out the progress box
		ProgressBox progress("", "Rendering...", total_length, 1);
		progress.start();

		current_label = mwindow->timebar->labels->first;
		while(!done && !result)
		{
// Get the length of this segment.
			if(list_mode)
			{
				while(current_label && current_label->position <= start)
					current_label = current_label->next;

				if(!current_label)
				{
					end = total_end;
					done = 1;
				}
				else
				{
					end = current_label->position;
				}

				if(end >= total_end)
				{
					done = 1;
					end = total_end;
				}
			}
			else
				done = 1;       // Not in list mode

// Insert the number in the filename.
			inject_number(asset, current_number);
// test existance of file
			{
				NewOrAppend window;
				result = window.test_file("", &asset);
				switch(result)
				{
					case 0:
						mwindow->purge_asset(asset.path);
						remove(asset.path);        // overwrite
						break;

					case 1:
						in_progress = 0;
						progress.stop_progress();     // mandatory
						return;             // cancel
						break;

					case 2:                 // append
						break;
				}
			}

// open output file
// use rd and wr to allow append
			file.set_processors(mwindow->preferences->smp > 2 ? (mwindow->preferences->smp - 1) : 1);
			result = file.try_to_open_file(mwindow->plugindb, &asset, 1, 1);

			if(!result)
			{
				file.seek_end();
//			if(do_video) startsource_frame = file.get_video_position();
//			if(do_audio) startsource_sample = file.get_audio_position();
// Extending files is disabled
// get_audio_position was broken for wav
				startsource_frame = 0;
				startsource_sample = 0;
// dithering is done at the file for rendering and is based in a 24 bit input resolution
				if(dither) file.set_dither();
			}
			else
			{
// open failed
				sprintf(string, "Couldn't open %s", asset.path);
				progress.stop_progress();     // mandatory
				ErrorBox error;
				error.create_objects(string);
				error.run_window();
				in_progress = 0;
				return;
			}

// figure out the length to read at a time
			long audio_read_length = mwindow->sample_rate;
			long video_read_length = 1;
			if(!video_read_length)
			{
				video_read_length = 1;
				audio_read_length = tosamples(video_read_length, asset.rate, asset.frame_rate);
			}

// Update the progress box		
			{
				char filename[1024];
				fs.extract_name(filename, asset.path);
				sprintf(string, "Rendering %s", filename);
				progress.update_title(string);
				progress.update_length(end - start);
			}

			{
// start the render engine
//printf("1\n");
				RenderEngine engine(mwindow);

//printf("1\n");
				engine.arm_playback_common(start,
								end,
								start,
								0,
								1,
								0,
								0);

//printf("1\n");
				if(do_audio)
					engine.arm_playback_audio(audio_read_length, 
								4096, 
								audio_read_length, 
								audio_read_length, 
								mwindow->output_channels);

//printf("1\n");
				if(do_video)
					engine.arm_playback_video(1,
								video_read_length + 1,  // need room for fractional frame
								1,
								mwindow->track_w,
								mwindow->track_h,
								mwindow->output_w,
								mwindow->output_h);

//printf("1\n");
// render_audio_position is the master determining what frame we're on
				long render_audio_position = start;
				long render_video_position = (long)toframes_round(start, asset.rate, asset.frame_rate);
				float **audio_output;
// (VFrame*)(VFrame array [])(Track [])
				VFrame ***video_output;

//printf("1\n");
// Create output buffers
				if(do_audio)
				{
					file.start_audio_thread(audio_read_length, mwindow->preferences->smp ? 2 : 1);
				}

//printf("1\n");
				if(do_video)
				{
					VideoConfig config = *mwindow->preferences->vconfig;
					config.video_out_driver = PLAYBACK_X11;
					compressed_frame = new VFrame;
					file.start_video_thread(1, 1, mwindow->preferences->smp ? 2 : 1, 0);
					video_device = new VideoDevice(mwindow);
					video_device->open_output(&config, 
						mwindow->frame_rate, 
						mwindow->output_w, 
						mwindow->output_h, 
						OUTPUT_RGB,
						0);
					video_device->start_playback();
				}

//printf("1\n");
				playable_tracks = new PlayableTracks(mwindow, 0, 0, TRACK_VIDEO);
				last_buffer = 0;
				direct_frame_copying = 0;

//printf("2\n");
// ================================== render it
				while(render_audio_position < end && !result)
				{
//printf("2\n");
					if(render_audio_position + audio_read_length > end)
					{
						last_buffer = 1;
						audio_read_length = end - render_audio_position;
					}

//printf("2\n");
					if(do_audio)
					{
						audio_output = file.get_audio_buffer();
						engine.arender->process_buffer(audio_output, audio_read_length, render_audio_position, last_buffer);
						result = file.write_audio_buffer(audio_read_length);
					}

//printf("2\n");
					render_audio_position += audio_read_length;
					if(do_video)
					{
// get the absolute video position from the audio position
						long video_end = (long)toframes(render_audio_position, 
													asset.rate, 
													asset.frame_rate);

						int last_video_buffer = 0;

//printf("2\n");
						while(render_video_position < video_end && !result)
						{
							if(render_video_position == video_end - 1) last_video_buffer = last_buffer;

//printf("2\n");
// Try to copy the compressed frame directly from the input to output files
							if(direct_frame_copy(render_video_position, &file))
							{
// Direct frame copy failed.
								if(direct_frame_copying)
								{
// Reenable background compression
									file.start_video_thread(1, 1, mwindow->preferences->smp ? 2 : 1, 0);
									direct_frame_copying = 0;
								}

//printf("2\n");
// Try to use the rendering engine to write the frame.
//printf("3\n");
// Get a buffer for background writing.
 								video_output = file.get_video_buffer();
//printf("3\n");
 								engine.vrender->process_buffer(video_output[0], 1, render_video_position, last_video_buffer);
//printf("3\n");
// Flash the frame to video out
 								if(video_device->output_visible()) video_device->write_buffer(video_output[0][0]);
//printf("3\n");
// Write to file
 								result = file.write_video_buffer(1, mwindow->preferences->video_use_alpha, mwindow->preferences->video_floatingpoint);
							}

							render_video_position++;
							if(!result) result = progress.cancelled();
						}
					}

					if(result && !progress.cancelled())
					{
						ErrorBox error;
						error.create_objects("Error writing data.");
						error.run_window();
					}
					else
					{
						result = progress.update(render_audio_position - start);
					}
				}

				engine.stop_playback();

				delete playable_tracks;

// Delete output buffers
				if(do_audio)
				{
// stop file I/O
					file.stop_audio_thread();
				}

				if(do_video)
				{
					delete compressed_frame;
					file.stop_video_thread();
					video_device->stop_playback();
					video_device->close_all();
					delete video_device;
				}
			}

			long endsource_sample;
			long endsource_frame;
			if(do_audio) endsource_sample = file.get_audio_length();
			if(do_video) endsource_frame = file.get_video_length(mwindow->frame_rate);
			if(!do_audio)
			{
				startsource_sample = tosamples(startsource_frame, asset.rate, asset.frame_rate);
				endsource_sample = tosamples(endsource_frame, asset.rate, asset.frame_rate);
			}

			file.close_file();

			if(!result && to_tracks)
			{             // paste it in
				mwindow->undo->update_undo_edits("Render", 0);

				result = mwindow->paste_output(start, 
												end, 
												startsource_sample, 
												endsource_sample, 
												startsource_frame,
												endsource_frame, 
												&asset, 
												0);
				mwindow->undo->update_undo_edits();
				if(!result) mwindow->changes_made = 1;
			}

			if(list_mode)
			{
				start = end;
				current_number++;
				file_number++;
			}
		} // file_number
		progress.stop_progress();     // mandatory
	} // !result
	in_progress = 0;
}

int Render::inject_number(Asset &asset, int current_number)
{
	int i, j, k;
	int len = strlen(asset.path);
	char printf_string[1024];
	int found_number = 0;

	if(list_mode)
	{
		for(i = 0, j = 0; i < number_start; i++, j++)
		{
				printf_string[j] = asset.path[i];
		}

// Found the number
		sprintf(&printf_string[j], "%%0%dd", total_digits);
		j = strlen(printf_string);
		i += total_digits;

// Copy remainder of string
		for( ; i < len; i++, j++)
		{
			printf_string[j] = asset.path[i];
		}
		printf_string[j] = 0;
// Print the printf argument to the path
		sprintf(asset.path, printf_string, current_number);
	}
return 0;
}

int Render::check_numbering(Asset &asset)
{
	int result = 0;
	char number_text[1024];
	int i, j = 0;
	int len = strlen(asset.path);

	total_digits = 0;
	number_start = 0;

	if(list_mode)
	{
// Search for last number
		for(i = len - 1; i >= 0; i--)
		{
			if(isdigit(asset.path[i]))
				break;
		}
		
// Search for beginning of last number
		while(i >= 0 && isdigit(asset.path[i]))
		{
			total_digits++;
			i--;
		}

		i++;
		if(i >= 0 && isdigit(asset.path[i]))
			number_start = i;
		else
		{
			ErrorBox error_box;
			error_box.create_objects("List filenames must contain a starting number", "e.g. file001.mov");
			error_box.run_window();
			result = 1;
		}
		
		if(!result)
		{
// Store the first number
			j = 0;
			while(isdigit(asset.path[i]))
				number_text[j++] = asset.path[i++];
			number_text[j] = 0;
			current_number = atol(number_text);
		}
	}
	return result;
return 0;
}

// Try to copy the compressed frame directly from the input to output files
// Return 1 on failure and 0 on success
int Render::direct_frame_copy(long &render_video_position, File *file)
{
	Track *playable_track;
	Edit *playable_edit;
	long frame_size;
	int result = 0;

	if(direct_copy_possible(render_video_position, playable_track, playable_edit, file))
	{
		if(!direct_frame_copying)
		{
// Disable background compression
			file->stop_video_thread();
			direct_frame_copying = 1;
		}
		frame_size = ((VEdit*)playable_edit)->compressed_frame_size(render_video_position);
		if(!frame_size)
		{
			return 1;
		}
		
		compressed_frame->allocate_compressed_data(frame_size);
		result = ((VEdit*)playable_edit)->read_compressed_frame(compressed_frame, render_video_position);
		result = file->write_compressed_frame(compressed_frame);
		return result;
	}
	else
		return 1;
return 0;
}

int Render::direct_copy_possible(long current_position, 
				Track* playable_track,  // The one track which is playable
				Edit* &playable_edit, // The edit which is playing
				File *file)   // Output file
{
	int result = 1;
	int total_playable_tracks = 0;
	Track* current_track;
	Patch* current_patch;
	Auto* current_auto;
	int temp;

// Number of playable tracks must equal 1
	for(current_track = mwindow->tracks->first, current_patch = mwindow->patches->first;
		current_track && result; 
		current_track = current_track->next, current_patch = current_patch->next)
	{
		if(current_track->data_type == TRACK_VIDEO)
		{
			if(Render::playable_tracks->is_playable(current_track, current_patch, current_position, 0))
			{
				playable_track = current_track;
				total_playable_tracks++;
			}
		}
	}
	if(total_playable_tracks != 1) result = 0;

// Edit must have a source file
	if(result)
	{
		playable_edit = playable_track->edits->get_render_strategy(current_position, 0, temp);
		if(!playable_edit)
			result = 0;
	}

// Source file must be able to copy to destination file.
// Source file must be same size as project output.
	if(result)
	{
		if(!file->can_copy_from(playable_edit->asset, mwindow->output_w, mwindow->output_h))
			result = 0;
	}

// Test conditions mutual between vrender.C and this.
	if(result && !playable_track->direct_copy_possible(current_position, current_position + 1))
		result = 0;

	return result;
return 0;
}

int Render::load_defaults(Asset *asset)
{
	defaults->get("RENDERPATH", asset->path);
	to_tracks = defaults->get("RENDERTOTRACKS", 1);
	do_audio = defaults->get("RENDERAUDIO", 1);
	do_video = defaults->get("RENDERVIDEO", 1);

	char string[1024];
	sprintf(string, "WAV");
	defaults->get("RENDERFORMAT", string);
	sprintf(asset->compression, QUICKTIME_YUV2);
	defaults->get("RENDERCOMPRESSION", asset->compression);
	asset->quality = defaults->get("RENDERQUALITY", 100);

	File file;
	asset->format = file.strtoformat(mwindow->plugindb, string);

	asset->bits = defaults->get("RENDERBITS", 16);
	dither = defaults->get("RENDERDITHER", 0);
	asset->signed_ = defaults->get("RENDERSIGNED", 1);
	asset->byte_order = defaults->get("RENDERBYTEORDER", 1);
return 0;
}

int Render::save_defaults(Asset *asset)
{
//printf("Render::save_defaults 1\n");
	defaults->update("RENDERPATH", asset->path);
	defaults->update("RENDERTOTRACKS", to_tracks);
	defaults->update("RENDERAUDIO", do_audio);
	defaults->update("RENDERVIDEO", do_video);
//printf("Render::save_defaults 2\n");

	File file;
	defaults->update("RENDERFORMAT", file.formattostr(mwindow->plugindb, asset->format));
//printf("Render::save_defaults 3\n");

	defaults->update("RENDERCOMPRESSION", asset->compression);
	defaults->update("RENDERQUALITY", asset->quality);
	defaults->update("RENDERBITS", asset->bits);
	defaults->update("RENDERDITHER", dither);
	defaults->update("RENDERSIGNED", asset->signed_);
	defaults->update("RENDERBYTEORDER", asset->byte_order);
//printf("Render::save_defaults 4\n");
return 0;
}






RenderWindow::RenderWindow(Render *render, Asset *asset)
 : BC_Window("", MEGREY, ICONNAME ": Render", 410, 350, 410, 340)
{
	this->render = render;
	this->asset = asset;
}

RenderWindow::~RenderWindow()
{
	delete ok_button;
	delete cancel_button;
	delete to_tracks_button;
	delete format_tools;
}



int RenderWindow::create_objects()
{
	int x = 5, y = 5;
	add_tool(new BC_Title(x, y, 
		(char*)(render->list_mode ? "Select the first file to render to:" : "Select a file to render to:")));
	y += 25;

	format_tools = new FormatTools(this, 
					render->mwindow->plugindb, 
					asset, &(render->do_audio),
					&(render->do_video),
					&(render->dither));
	format_tools->create_objects(x, y, 1, 1, 1, 1, 0);

	y += 250;
	add_tool(to_tracks_button = new RenderToTracks(x, y, render, render->to_tracks));
	y += 25;
	x = 80;
	add_tool(ok_button = new RenderOK(x, y));
	x += 200;
	add_tool(cancel_button = new RenderCancel(x, y));
return 0;
}






RenderOK::RenderOK(int x, int y) : BC_BigButton(x, y, "Make it so!") {}
RenderOK::~RenderOK() {};
int RenderOK::handle_event() { set_done(0); return 0;
};
int RenderOK::keypress_event() 
{ 
	if(get_keypress() == 13) { handle_event(); return 1; }
	return 0;
return 0;
}

RenderCancel::RenderCancel(int x, int y) : BC_BigButton(x, y, "Cancel") {}
RenderCancel::~RenderCancel() {};
int RenderCancel::handle_event() { set_done(1); return 0;
};
int RenderCancel::keypress_event() 
{ 
	if(get_keypress() == ESC) { handle_event(); return 1; }
	return 0;
return 0;
}



RenderToTracks::RenderToTracks(int x, int y, Render *render, int default_)
 : BC_CheckBox(x, y, 17, 17, default_, "Overwrite project with output")
 { this->render = render; }
RenderToTracks::~RenderToTracks() {}
int RenderToTracks::handle_event()
{
	render->to_tracks = get_value();
return 0;
}
