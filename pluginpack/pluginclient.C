#include <string.h>
#include "pluginclient.h"



PluginClient::PluginClient(int argc, char *argv[])
{
	interactive = 0;
	gui_messages = 0;
	show_initially = 0;
	message_lock = 0;
	wr = rd = 0;
	master_gui_on = 0;
	client_gui_on = 0;

	plugin_cleanup();

	plugin_init(argc, argv);
}

PluginClient::~PluginClient()
{
	plugin_exit();
	if(gui_messages) delete gui_messages;
}

int PluginClient::plugin_exit()
{
	stop_gui_client();
	if(total_in_buffers)
	{
		for(int i = 0; i < total_in_buffers; i++)
		{
			delete data_in[i];
		}
		delete data_in;
	}

	if(total_out_buffers)
	{
		for(int i = 0; i < total_out_buffers; i++)
		{
			delete data_out[i];
		}
		delete data_out;
	}

// some plugins are not child processes so send a confirmation
	if(messages) 
	{
		messages->write_message(COMPLETED);
		delete messages;
	}
	if(message_lock) delete message_lock;

	plugin_cleanup();
	exit(0);
}

int PluginClient::plugin_cleanup()
{
	total_in_buffers = 0;
	total_out_buffers = 0;
	messages = 0;
	message_lock = 0;
}

int PluginClient::plugin_init(int argc, char *argv[])
{
	if(argc < 3)
	{
		printf("This is a plugin for Broadcast 2000a.  Grow a mane.\n");
		success = 0;
		return 1;
	}

// get the message pipes
	messages = new Messages(MESSAGE_TO_PLUGIN, MESSAGE_FROM_PLUGIN, atol(argv[1]));

	if(atol(argv[2]) >= 0)
	{
		gui_messages = new Messages(MESSAGE_TO_PLUGIN, MESSAGE_FROM_PLUGIN, atol(argv[2]));
		gui_messages->write_message(GET_STRING);
		gui_messages->read_message(gui_string);

		show_initially = 1;       // flag for plugin
		client_gui_on = 1;
	}
	success = 1;

	return 0;
}

int PluginClient::plugin_get_range()
{
	messages->read_message(&start, &end);
}

// For non realtime plugins, allocate all the buffers.
int PluginClient::plugin_negotiate_buffers()
{
	long recommended_size;
	long total_channels;

	messages->read_message(&total_channels, &recommended_size);
	total_in_buffers = total_out_buffers = total_channels;

// get desired sizes from user
	in_buffer_size = get_in_buffers(recommended_size);
	out_buffer_size = get_out_buffers(recommended_size);
// notify server of desired sizes
	messages->write_message(out_buffer_size, in_buffer_size);

// get shared buffers from server
	data_in = new PluginBuffer*[total_in_buffers];
	data_out = new PluginBuffer*[total_out_buffers];

	long mem_id, size;
// Sizes recieved from server are now byte counts.
	for(int i = 0; i < total_in_buffers; i++)
	{
		messages->read_message(&mem_id, &size);
		data_in[i] = new PluginBuffer(mem_id, size, 1);
	}

	for(int i = 0; i < total_out_buffers; i++)
	{
		messages->read_message(&mem_id, &size);
		data_out[i] = new PluginBuffer(mem_id, size, 1);
	}
	return 0;
}

int PluginClient::plugin_restart_realtime()
{
	plugin_delete_buffers();

// Defeat an extra parameter from PluginServer::send_buffer_info
	long lock_id;
	lock_id = messages->read_message();

	plugin_create_buffers();
	return 0;
}

// For realtime plugins initialize buffers
int PluginClient::plugin_init_realtime()
{
// Get parameters for all
	master_gui_on = get_gui_status();

// get parameters depending on video or audio
	init_realtime_parameters();
	smp = get_project_smp();

	messages->write_message(GET_REALTIME_BUFFERS);

// need long sized variables for read message
	long lock_id;
	lock_id = messages->read_message();
	message_lock = new Sema(lock_id);

	plugin_create_buffers();

// user initialization
	start_realtime();
	
	messages->write_message(COMPLETED);
	return 0;
}

int PluginClient::plugin_create_buffers()
{
// need long sized variables for read message
	long total_channels, size;

	messages->read_message(&total_channels, &size);
	total_in_buffers = total_out_buffers = total_channels;
	in_buffer_size = out_buffer_size = size;

// get input buffers from server
	long mem_id, double_buffers, buffer_size;
	int i, j;
	for(i = 0; i < total_in_buffers; i++)
	{
// get number of double buffers for this channel
		messages->read_message(&double_buffers, &buffer_size);
		double_buffers_in.append(double_buffers);
		realtime_in_size.append(buffer_size);
		offset_in_render.append(0);
		double_buffer_in_render.append(0);
		data_in_realtime.append(new PluginBuffer*[double_buffers]);
		for(j = 0; j < double_buffers; j++)
		{
			mem_id = messages->read_message();
// word size is unused in PluginBuffer
			data_in_realtime.values[i][j] = new PluginBuffer(mem_id, size, 1);
//printf("data_in_realtime.values[%d][%d] = %x\n", i, j, data_in_realtime.values[i][j]->get_data());
		}
	}

// get output buffers
	for(i = 0; i < total_out_buffers; i++)
	{
		messages->read_message(&double_buffers, &buffer_size);
		double_buffers_out.append(double_buffers);
		realtime_out_size.append(buffer_size);
		offset_out_render.append(0);
		double_buffer_out_render.append(0);
		data_out_realtime.append(new PluginBuffer*[double_buffers]);
		for(j = 0; j < double_buffers; j++)
		{
			mem_id = messages->read_message();
// word size is unused in PluginBuffer
			data_out_realtime.values[i][j] = new PluginBuffer(mem_id, size, 1);
//printf("data_out_realtime.values[%d][%d] = %x\n", i, j, data_out_realtime.values[i][j]->get_data());
		}
	}
	
// initialize the proper data types
	create_buffer_ptrs();
	return 0;
}

int PluginClient::plugin_stop_realtime()
{
// give to user
	stop_realtime();
	int i, j;

	plugin_delete_buffers();
	delete message_lock;
	message_lock = 0;
	return 0;
}

int PluginClient::plugin_delete_buffers()
{
	int i, j;
// used in plugin that also runs the GUI
	delete_buffer_ptrs();
	if(total_in_buffers)
	{
		for(i = 0; i < total_in_buffers; i++)
		{
			for(j = 0; j < double_buffers_in.values[i]; j++)
			{
				delete data_in_realtime.values[i][j];
			}
			delete data_in_realtime.values[i];
		}
		offset_in_render.remove_all();
		double_buffer_in_render.remove_all();
		data_in_realtime.remove_all();
		double_buffers_in.remove_all();
		realtime_in_size.remove_all();
	}
	
	if(total_out_buffers)
	{
		for(i = 0; i < total_out_buffers; i++)
		{
			for(j = 0; j < double_buffers_out.values[i]; j++)
			{
				delete data_out_realtime.values[i][j];
			}
			delete data_out_realtime.values[i];
		}
		offset_out_render.remove_all();
		double_buffer_out_render.remove_all();
		data_out_realtime.remove_all();
		double_buffers_out.remove_all();
		realtime_out_size.remove_all();
	}

	total_in_buffers = 0;
	total_out_buffers = 0;
	return 0;
}

int PluginClient::plugin_get_parameters()
{
	int result = get_parameters();
	messages->write_message(result);
	return 0;
}

// ========================= main loop
int PluginClient::plugin_run()
{
	if(!success) return 1;

	int command;
	int done = 0;

	if(client_gui_on)
		start_gui();         // Start the GUI

	while(!done)
	{
		command = messages->read_message();

		switch(command)
		{
			case -1:                    done = 1;                                                 break;
			case START_REALTIME:        plugin_init_realtime();                                   break;
			case STOP_REALTIME:         plugin_stop_realtime();                                   break;
			case PROCESS_REALTIME:      plugin_process_realtime();                                break;
			case RESTART_REALTIME:      plugin_restart_realtime();                                break;
			case GET_AUDIO:             messages->write_message(plugin_is_audio());               break;
			case GET_VIDEO:             messages->write_message(plugin_is_video());               break;
			case GET_REALTIME:          messages->write_message(plugin_is_realtime());            break;
			case GET_MULTICHANNEL:      messages->write_message(plugin_is_multi_channel());       break;
			case GET_FILEIO:            messages->write_message(plugin_is_fileio());              break;
			case GET_TITLE:             messages->write_message(plugin_title());                  break;
			case EXIT_PLUGIN:           plugin_exit(); done = 1;                                  break;
			case LOAD_DEFAULTS:         load_defaults();                                          break;
			case SAVE_DEFAULTS:         save_defaults();                                          break;
			case STOP_GUI:              stop_gui_client();                                        break;
			case HIDE_GUI:              hide_gui(); 	                                          break;
			case SET_STRING:            set_string_client();                                      break;
			case GET_PARAMETERS:        plugin_get_parameters();                                  break;
			case GET_SAMPLERATE:        messages->write_message(get_plugin_samplerate());         break;
			case GET_FRAMERATE:         messages->write_message((long)(get_plugin_framerate() * 1000));break;
			case SET_INTERACTIVE:       interactive = 1;                                          break;
			case SET_RANGE:             plugin_get_range();                                       break;
			case GET_BUFFERS:           plugin_negotiate_buffers();                               break;
			case START_PLUGIN:          plugin_start_plugin();                                    break;
			case SAVE_DATA:             save_data_client();                                       break;
			case LOAD_DATA:             load_data_client();                                       break;
			case CONFIGURE_CHANGE:      get_configure_change();                                   break;
			case GUI_ON:                master_gui_on = 1;                                        break;
			case GUI_OFF:               master_gui_on = 0;                                        break;
			default:                    plugin_command_derived(command);                          break;
		}
	}
	return 0;
}

int PluginClient::start_realtime() { printf("start_realtime\n"); }
int PluginClient::stop_realtime() { printf("stop_realtime\n"); }
int PluginClient::plugin_is_realtime() { return 0; }
int PluginClient::plugin_is_multi_channel() { return 0; }
int PluginClient::plugin_is_audio() { return 0; }
int PluginClient::plugin_is_video() { return 0; }
int PluginClient::plugin_is_fileio() { return 0; }
int PluginClient::create_buffer_ptrs() { return 0; }
int PluginClient::delete_buffer_ptrs() { return 0; }
char* PluginClient::plugin_title() { return "Untitled"; }
int PluginClient::is_realtime() { return 0; }
int PluginClient::is_audio() { return 0; }
int PluginClient::is_video() { return 0; }
int PluginClient::is_fileio() { return 0; }
int PluginClient::is_multichannel() { return 0; }

int PluginClient::load_defaults() { return 0; }
int PluginClient::save_defaults() { return 0; }
int PluginClient::start_gui() { return 0; }
int PluginClient::stop_gui() { return 0; }
int PluginClient::show_gui() { return 0; }
int PluginClient::hide_gui() { return 0; }
int PluginClient::save_data(char *text) { return 0; }
int PluginClient::read_data(char *text) { return 0; }
int PluginClient::set_string() { return 0; }
int PluginClient::get_parameters() { return 0; }
int PluginClient::get_plugin_samplerate() { return -1; }
float PluginClient::get_plugin_framerate() { return -1; }
int PluginClient::init_realtime_parameters() { return 0; }
int PluginClient::init_nonrealtime_parameters() { return 0; }
int PluginClient::delete_nonrealtime_parameters() { return 0; }
int PluginClient::process_realtime(long size) 
{
	printf("No realtime processor defined for this plugin.\n"); 
}

long PluginClient::get_in_buffers(long recommended_size)
{
	return recommended_size;
}

long PluginClient::get_out_buffers(long recommended_size)
{
	return recommended_size;
}

int PluginClient::get_gui_status()
{
	messages->write_message(GET_GUI_STATUS);
	return messages->read_message() == GUI_ON;
}

int PluginClient::plugin_start_plugin()
{
// Create any necessary pointers and download any parameters for the derived plugin.
	init_nonrealtime_parameters();
	smp = get_project_smp();

	start_plugin();

	delete_nonrealtime_parameters();
	send_completed();
	return 0;
}

int PluginClient::start_plugin()
{
	printf("No processing defined for this plugin.\n");
	return 0;
}

// communication convenience routines

int PluginClient::stop_gui_client()
{
	if(!client_gui_on) return 0;
	client_gui_on = 0;
	stop_gui();                      // give to plugin
	gui_messages->write_message(COMPLETED);
	delete gui_messages;
	return 0;
}


int PluginClient::set_string_client()
{
	messages->read_message(gui_string);
	set_string();
	return 0;
}

int PluginClient::save_data_client()
{
	save_data(messages->get_message_buffer());         // user puts data directly into buffer
	messages->write_message_raw();
	return 0;
}

int PluginClient::load_data_client()
{
	read_data(messages->read_message_raw());         // user reads data directly from buffer
	return 0;
}

int PluginClient::plugin_process_realtime()
{
	long size, i;
	PluginClientAuto new_auto;

	messages->read_message(&size, &source_len, &source_position);
	for(i = 0; i < total_in_buffers; i++)
	{
		messages->read_message(&(offset_in_render.values[i]), &(double_buffer_in_render.values[i]));
	}
	for(i = 0; i < total_in_buffers; i++)
	{
		messages->read_message(&(offset_out_render.values[i]), &(double_buffer_out_render.values[i]));
	}

	messages->read_message_f(&new_auto.position, &new_auto.intercept, &new_auto.slope);

//printf("PluginClient::plugin_process_realtime %f %f\n", new_auto.intercept, new_auto.slope);
	while(new_auto.position != -1)
	{
		automation.append(new_auto);
		messages->read_message_f(&new_auto.position, &new_auto.intercept, &new_auto.slope);
	}
	
	process_realtime(size);
	
	automation.remove_all();
	messages->write_message(COMPLETED);
	return 0;
}

int PluginClient::automation_used()    // If automation is used
{
	if(automation.total) return 1;
	else
	return 0;
}

float PluginClient::get_automation_value(long position)     // Get the automation value for the position in the current fragment
{
	int i;
	for(i = automation.total - 1; i >= 0; i--)
	{
		if(automation.values[i].position <= position)
		{
			return automation.values[i].intercept + automation.values[i].slope * (position - automation.values[i].position);
		}
	}
	return 0;
}

int PluginClient::buffers_identical(int channel)
{
	if(data_in_realtime.values[channel][double_buffer_in_render.values[channel]]->shmid
		== data_out_realtime.values[channel][double_buffer_out_render.values[channel]]->shmid)
		return 1;
	else
		return 0;
}

long PluginClient::get_source_len()
{
	return source_len;
}

long PluginClient::get_source_position()
{
	return source_position;
}

int PluginClient::get_configure_change()
{
	read_data(messages->read_message_raw());
	return 0;
}


long PluginClient::get_project_samplerate()
{
	int result;
	messages->write_message(GET_SAMPLERATE);
	result = messages->read_message();
	return result;
}

float PluginClient::get_project_framerate()
{
	float result;
	messages->write_message(GET_FRAMERATE);
	result = (float)messages->read_message() / 1000;
	return result;
}

int PluginClient::get_project_smp()
{
	messages->write_message(GET_SMP);
	return messages->read_message();
}

int PluginClient::get_gui_visible()
{
	int result;
	messages->write_message(GET_GUI_STATUS);
	result = messages->read_message();
	return result;
}

int PluginClient::get_project_framesize(int &frame_w, int &frame_h)
{
	long w, h;
	messages->write_message(GET_FRAMESIZE);
	messages->read_message(&w, &h);
	
	frame_w = w;
	frame_h = h;
	return 0;
}

char* PluginClient::get_defaultdir()
{
	return BCASTDIR;
}

int PluginClient::get_use_float()
{
	int result;
	messages->write_message(GET_USE_FLOAT);
	result = messages->read_message();
	return result;
}

int PluginClient::get_use_alpha()
{
	int result;
	messages->write_message(GET_USE_ALPHA);
	result = messages->read_message();
	return result;
}

int PluginClient::get_use_interpolation()
{
	int result;
	messages->write_message(GET_USE_INTERPOLATION);
	result = messages->read_message();
	return result;
}

int PluginClient::get_aspect_ratio(float &aspect_w, float &aspect_h)
{
	long result1, result2;
	messages->write_message(GET_ASPECT_RATIO);
	messages->read_message(&result1, &result2);
	aspect_w = (float)result1;
	aspect_h = (float)result2;
	return 0;
}

int PluginClient::send_completed()
{
	messages->write_message(COMPLETED);
}

int PluginClient::send_cancelled()
{
	messages->write_message(CANCEL);
}

int PluginClient::send_hide_gui()
{
// Stop the GUI server and delete GUI messages
	if(!client_gui_on) return 0;
	client_gui_on = 0;
	gui_messages->write_message(COMPLETED);
	delete gui_messages;
}

int PluginClient::send_configure_change()
{
// handle everything using the gui messages
	gui_messages->write_message(CONFIGURE_CHANGE);
	save_data(gui_messages->get_message_buffer());         // user puts data directly into buffer
	gui_messages->write_message_raw();     // send the data
}

int PluginClient::read_frames(long start_position, long total_frames)
{
	messages->write_message(READ_FRAMES);
	messages->write_message(total_frames, start_position);
// buffers are loaded by server
	int result = messages->read_message();      // gets ok or cancel
	if(result == OK) return 0;
	else return 1;
}

int PluginClient::write_frames(long total_frames)
{
// buffers are preloaded by client
	messages->write_message(WRITE_FRAMES);

	messages->write_message(total_frames);
	int result = messages->read_message();
	if(result == OK) return 0;
	else return 1;
}

int PluginClient::read_samples(long start_position, long total_samples)
{
	messages->write_message(READ_SAMPLES);
	messages->write_message(total_samples, start_position);
// buffers are loaded by server
	int result = messages->read_message();      // gets ok or cancel
	if(result == OK) return 0;
	else return 1;
}

int PluginClient::write_samples(long total_samples)
{
// buffers are preloaded by client
	messages->write_message(WRITE_SAMPLES);
	
	messages->write_message(total_samples);
	int result = messages->read_message();
	if(result == OK) return 0;
	else return 1;
}
