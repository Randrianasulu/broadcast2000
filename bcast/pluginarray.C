#include <string.h>
#include "atrack.h"
#include "cache.h"
#include "file.h"
#include "mainwindow.h"
#include "pluginarray.h"
#include "pluginbuffer.h"
#include "pluginserver.h"
#include "preferences.h"
#include "progressbox.h"




PluginArray::PluginArray(MainWindow *mwindow)
 : ArrayList<PluginServer*>()
{
	this->mwindow = mwindow;
	multichannel = 0;
}

PluginArray::~PluginArray()
{
}

int PluginArray::start_plugins(PluginServer *old_plugin)
{
	PluginServer *plugin;
	int i;

	realtime = 0;
	if(!multichannel)
	{
// ============================ single channel plugins
// start 1 plugin for each track
		for(i = 0; i < total_tracks(); i++)
		{
			append(plugin = new PluginServer(*old_plugin));
			plugin->set_mainwindow(mwindow);
			plugin->set_track(track_number(i));
			plugin->open_plugin();
			plugin->load_defaults();
			plugin->negotiate_buffers(get_bufsize());
		}
	}
	else
	{
// ============================ multichannel
// start 1 plugin for all tracks
		append(plugin = new PluginServer(*old_plugin));
		plugin->set_mainwindow(mwindow);

		for(i = 0; i < total_tracks(); i++)
		{
			plugin->set_track(track_number(i));
		}

		plugin->open_plugin();
		plugin->load_defaults();
		plugin->negotiate_buffers(get_bufsize());
	}

// set one plugin for progress bars
	values[total - 1]->set_interactive();
//printf("PluginArray::start_plugins 8\n");
return 0;
}

// =================================== run realtime plugins only
int PluginArray::start_realtime_plugins(PluginServer *old_plugin, char *plugin_data)
{
	PluginServer *plugin;
	int i;

	realtime = 1;
	buffer_size = get_bufsize();
	realtime_buffers = new PluginBuffer*[total_tracks()];
	if(!multichannel)
	{
// single channel plugins
// start 1 plugin for each track
		for(i = 0; i < total_tracks(); i++)
		{
			realtime_buffers[i] = get_buffer();

			append(plugin = new PluginServer(*old_plugin));
			plugin->set_mainwindow(mwindow);
			plugin->open_plugin();
			plugin->attach_input_buffer(&realtime_buffers[i], 1, get_bufsize(), get_bufsize());
			plugin->attach_output_buffer(&realtime_buffers[i], 1, get_bufsize(), get_bufsize());
			plugin->init_realtime();
			plugin->get_configuration_change(plugin_data);				
		}
	}
	else
	{
// multichannel
// start 1 plugin for all tracks
		append(plugin = new PluginServer(*old_plugin));
		plugin->set_mainwindow(mwindow);
		plugin->open_plugin();

		for(i = 0; i < total_tracks(); i++)
		{
			realtime_buffers[i] = get_buffer();

			plugin->attach_input_buffer(&realtime_buffers[i], 1, get_bufsize(), get_bufsize());
			plugin->attach_output_buffer(&realtime_buffers[i], 1, get_bufsize(), get_bufsize());
		}

		plugin->init_realtime();
		plugin->get_configuration_change(plugin_data);				
	}
return 0;
}

int PluginArray::set_range(long start, long end)
{
	if(realtime)
	{
		realtime_start = start;
		realtime_end = end;
	}
	else
	{
		for(int i = 0; i < total; i++)
		{
			values[i]->set_range(start, end);
		}
	}
return 0;
}

int PluginArray::set_file(File *file)
{
	this->file = file;
return 0;
}

int PluginArray::set_multichannel()
{
	multichannel = 1;
return 0;
}

// ========================= run non-realtime plugins only
int PluginArray::run_plugins()
{
	int i, j, result;
	int done = 0;     // for when done
	int write_samples = 0, write_frames = 0; // Count of plugins which have written a buffer.
	long samples_written, frames_written;  // samples written during last write
	int error = 0;

	start_plugins_derived();

// handle one request from each plugin at a time
// plugins must all finish at same time
	while(!done && !error)
	{
		for(i = 0; i < total; i++)
		{
			result = values[i]->handle_plugin_command();

			switch(result)
			{
				case 0:
// do nothing
					break;

				case 1:
// finished
					done = 1;
					break;

				case 2:
// signal event handler to write samples
					if(values[i]->audio)
					{
						samples_written = values[i]->get_written_samples();
						write_samples++;
					}
					else
					if(values[i]->video)
					{
						frames_written = values[i]->get_written_frames();
						write_frames++;
					}
					break;

				case 3:
// cancel
					done = 1;
					error = 1;
					break;
			}
		}

// samples written from each plugin
// The counter must equal the total number of plugins before writing.
		if(write_samples == total)
		{
			result = write_samples_derived(samples_written);
// Assume all plugins wrote at the same time.
			for(i = 0; i < total; i++) values[i]->send_write_result(result);
			if(result)
			{
				error = 1;
			}
			write_samples = 0;
		}

		if(write_frames == total)
		{
			result = write_frames_derived(frames_written);
// Assume all plugins wrote at the same time.
			for(i = 0; i < total; i++) values[i]->send_write_result(result);
			if(result)
			{
				error = 1;
			}
			write_frames = 0;
		}

		if(error)
// send all plugins a cancel to stop waiting
		{
			for(i = 0; i < total; i++) values[i]->send_cancel();
		}
	}

	return error;
return 0;
}

int PluginArray::run_realtime_plugins(char *title)
{
// run realtime plugins only
	ProgressBox progress("", title, realtime_end - realtime_start, 1);
	progress.start();

	long fragment_len = buffer_size;
	long source_length = realtime_end - realtime_start;
	int result = 0;
	int i, j;

	start_realtime_plugins_derived();

	for(long position = realtime_start; position < realtime_end && !result; )
	{
		fragment_len = buffer_size;
		if(position + fragment_len > realtime_end) fragment_len = realtime_end - position;

// arm buffers
		for(i = 0; i < total_tracks(); i++)
		{
			render_track(i, fragment_len, position);

			if(multichannel)
			{
				values[0]->arm_buffer(i, 0, 0, 0, 0);
			}
			else
			{
				values[i]->arm_buffer(0, 0, 0, 0, 0);
			}
		}

		mwindow->cache->age_audio();
		mwindow->cache->age_video();

// process buffers
		for(i = 0; i < (multichannel ? 1 : total); )
		{
// Only run up to mwindow->smp + 1 plugins at a time
			if(!multichannel)
			{
				for(j = 0; j < mwindow->preferences->smp + 1 && j + i < total; j++)
				{
					values[i + j]->process_realtime_start(source_length, position - realtime_start, fragment_len);
				}

				for(j = 0; j < mwindow->preferences->smp + 1 && i < total; j++, i++)
				{
					values[i]->process_realtime_end();
				}
			}
			else
			{
// Only 1 plugin for a multichannel.
				values[i]->process_realtime(source_length, position - realtime_start, fragment_len);
				i++;
			}
		}

// write_buffers
		result = write_samples_derived(fragment_len);

		position += fragment_len;
		if(!result)
		{
			result = progress.update(position - realtime_start);
		}
	}

	progress.stop_progress();
	return result;
return 0;
}

int PluginArray::stop_plugins()
{
	stop_plugins_derived();
	while(total)
	{
		values[0]->close_plugin();
		delete values[0];
		remove_number(0);
	}
	realtime = 0;
return 0;
}

int PluginArray::stop_realtime_plugins()
{
	stop_plugins_derived();
	if(multichannel)
	{
		values[0]->realtime_stop();
		values[0]->close_plugin();
		delete values[0];
		remove_number(0);

		for(int i = 0; i < total_tracks(); i++)
		{
			delete realtime_buffers[i];
		}
		delete realtime_buffers;
	}
	else
	{
		for(int i = 0; i < total_tracks(); i++)
		{
			values[i]->realtime_stop();
			values[i]->close_plugin();
			delete values[i];
			remove_number(i);
			delete realtime_buffers[i];
		}
		delete realtime_buffers;
	}
	realtime = 0;
return 0;
}
