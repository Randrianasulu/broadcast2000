#include <string.h>
#include "atrack.h"
#include "attachmentpoint.h"
#include "floatautos.h"
#include "mainwindow.h"
#include "menueffects.h"
#include "messages.h"
#include "neworappend.h"
#include "plugin.h"
#include "pluginbuffer.h"
#include "plugincommands.h"
#include "pluginserver.h"
#include "preferences.h"
#include "sema.h"
#include "vframe.h"
#include "vtrack.h"


#include <sys/types.h>
#include <sys/wait.h>



PluginGUIServer::PluginGUIServer()
 : Thread()
{
// PluginGUIServer must be able to exit by itself for a plugin close event
	synchronous = 0;
}

PluginGUIServer::~PluginGUIServer()
{
}

int PluginGUIServer::start_gui_server(PluginServer *plugin_server, char *string)
{
	this->plugin_server = plugin_server;
	messages = new Messages(MESSAGE_FROM_PLUGIN, MESSAGE_TO_PLUGIN);
	strcpy(this->string, string);
	completion_lock.lock();
	start();
return 0;
}

void PluginGUIServer::run()
{
	int done = 0;
	int command;
	
	while(!done)
	{
		command = messages->read_message();

		switch(command)
		{
			case COMPLETED:          
			{
				MenuEffectPrompt *prompt = plugin_server->prompt;
				done = 1;  						  
				plugin_server->gui_on = 0;
                if(plugin_server->plugin_open)
                {
			        if(prompt)
			        {
// Cancel a non realtime effect
				        prompt->set_done(1);
			        }
			        else
			        {
// Update the attachment point.
				        plugin_server->attachment->set_show(0);
			        }
                }
				done = 1;

                plugin_server->close_plugin();
			}
				break;

			case CONFIGURE_CHANGE:
// Propagate the configuration to the attachment point.
// non realtime context doesn't have an attachment
				if(plugin_server->attachment)
					plugin_server->attachment->get_configuration_change(messages->read_message_raw());
//printf("PluginGUIServer::run 2\n");
				break;

			case GET_STRING:
				messages->write_message(string);
				break;
		}
	}
	delete messages;
	completion_lock.unlock();
}




PluginForkThread::PluginForkThread() : Thread()
{
	synchronous = 0;    // detach the thread
}

PluginForkThread::~PluginForkThread()
{
}

int PluginForkThread::fork_plugin(char *path, char **args)
{
	this->path = path;
	this->args = args;
//	fork_lock.lock();
	completion_lock.lock();
	start();
// wait for process to start
// 	fork_lock.lock();
// 	fork_lock.unlock();
	return 0;
return 0;
}

int PluginForkThread::wait_completion()
{
	completion_lock.lock();
	completion_lock.unlock();
return 0;
}


void PluginForkThread::run()
{
// fork it
	plugin_pid = fork();

	if(plugin_pid == 0)
	{
		execv(path, args);     // turn into the plugin
	}
	else
	{
		int plugin_status;
//		sleep(1);
// Signal fork finished
//		fork_lock.unlock();
// Wait for plugin to finish
		if(waitpid(plugin_pid, &plugin_status, WUNTRACED) == -1)
		{
			perror("PluginForkThread::waitpid:");
		}
		completion_lock.unlock();
		return;            // wait for the plugin to finish and clean up
	}
}







PluginServer::PluginServer()
{
	reset_parameters();
	tracks = new ArrayList<Track*>;
	fork_thread = new PluginForkThread;
	gui_server = new PluginGUIServer;
}

PluginServer::PluginServer(char *path)
{
	reset_parameters();
	set_path(path);
	tracks = new ArrayList<Track*>;
	fork_thread = new PluginForkThread;
	gui_server = new PluginGUIServer;
}

PluginServer::PluginServer(PluginServer &that)
{
	reset_parameters();

	if(that.title)
	{
		title = new char[strlen(that.title) + 1];
		strcpy(title, that.title);
	}

	if(that.path)
	{
		path = new char[strlen(that.path) + 1];
		strcpy(path, that.path);
	}

	tracks = new ArrayList<Track*>;
	fork_thread = new PluginForkThread;
	gui_server = new PluginGUIServer;

	attachment = that.attachment;	
	realtime = that.realtime;
	multichannel = that.multichannel;
	audio = that.audio;
	video = that.video;
	fileio = that.fileio;
	mwindow = that.mwindow;
}

PluginServer::~PluginServer()
{
	if(path) delete path;
	if(title) delete title;
	delete tracks;
	if(message_lock) delete message_lock;
	delete fork_thread;
	delete gui_server;
}

int PluginServer::reset_parameters()
{
	cleanup_plugin();
	autos = 0;
	plugin_open = 0;
	title = 0;
	path = 0;
	audio = video = 0;
	realtime = multichannel = fileio = 0;
	start_auto = end_auto = 0;
return 0;
}

int PluginServer::dump()
{
	printf("			pluginserver %x\n", this);
	printf("				path %x title %x\n", path, title);
return 0;
}


int PluginServer::cleanup_plugin()
{
	in_buffer_size = out_buffer_size = 0;
	total_in_buffers = total_out_buffers = 0;
	messages = 0;
	message_lock = 0;
	error_flag = 0;
	written_samples = 0;
	shared_buffers = 0;
	new_buffers = 0;
	written_samples = written_frames = 0;
	prompt = 0;
	gui_on = 0;
	temp_frame_buffer = 0;
	temp_frame = 0;
return 0;
}

int PluginServer::set_mainwindow(MainWindow *mwindow)
{
	this->mwindow = mwindow;
return 0;
}

int PluginServer::set_path(char *path)
{
	if(this->path) delete this->path;
	this->path = new char[strlen(path) + 1];
	strcpy(this->path, path);
return 0;
}

int PluginServer::query_plugin()
{
// start plugin
	if(open_plugin()) return 1;

	realtime = get_realtime();
	audio = get_audio();
	video = get_video();
	fileio = get_fileio();
	multichannel = get_multichannel();

// don't want a title for every plugin server
// some plugin servers are used in the plugindb
// some are used for running plugins
	char string[MESSAGESIZE];
	get_title(string);
	if(title) delete title;
	title = new char[strlen(string) + 1];
	strcpy(title, string);

	close_plugin();
	return 0;
return 0;
}


// Basic plugin open
int PluginServer::open_plugin()
{
	if(plugin_open) return 0;
	plugin_open = 1;
	messages = new Messages(MESSAGE_FROM_PLUGIN, MESSAGE_TO_PLUGIN);

// fill arguments
	args[0] = new char[strlen(path) + 1];
	sprintf(args[0], "%s", path);
	args[1] = new char[32];
	sprintf(args[1], "%d", messages->get_id());
	args[2] = new char[32];

	if(gui_on)
		sprintf(args[2], "%d", gui_server->messages->get_id());
	else
		sprintf(args[2], "-1");

	args[3] = 0;
	total_args = 4;

// fork it
	fork_thread->fork_plugin(path, args);
	return 0;
}

// Open plugin with a GUI
int PluginServer::open_plugin(AttachmentPoint *attachment, MenuEffectPrompt *prompt, char *string)
{
	if(plugin_open) return 0;
	this->attachment = attachment;
	this->prompt = prompt;
	gui_on = 1;
	gui_server->start_gui_server(this, string);
	open_plugin();
	return 0;
}

int PluginServer::close_plugin()
{
	int plugin_status, result;
	if(!plugin_open) return 0;
	plugin_open = 0;

	messages->write_message(EXIT_PLUGIN);
	fork_thread->wait_completion();
	delete messages;

// delete pointers to shared buffers but not the shared buffers themselves
	if(data_in.total)
	{
		if(!shared_buffers)
			for(int i = 0; i < data_in.total; i++) delete data_in.values[i];
		data_in.remove_all();
	}

	if(data_out.total)
	{
		if(!shared_buffers)
			for(int i = 0; i < data_out.total; i++) delete data_out.values[i];
		data_out.remove_all();
	}

	if(temp_frame)
	{
		delete temp_frame;
		delete temp_frame_buffer;
	}

// these are always shared
	data_in_realtime.remove_all();
	data_out_realtime.remove_all();
	offset_in_render.remove_all();
	offset_out_render.remove_all();
	double_buffer_in_render.remove_all();
	double_buffer_out_render.remove_all();
	realtime_in_size.remove_all();
	realtime_out_size.remove_all();
	delete args[0];
	delete args[1];
	delete args[3];
	total_args = 0;

	cleanup_plugin();
return 0;
}

int PluginServer::plugin_server_loop()
{
	int done = 0;
	int command;
	if(!plugin_open) return 0;

	while(!done)
	{
		done = handle_plugin_command();
	}
	return 0;
return 0;
}


// return values
// 0 ok
// 1 finished
// 2 buffer written
// 3 cancel
int PluginServer::handle_plugin_command()
{
	int result;
	int command;
	if(!plugin_open) return 0;

	command = messages->read_message();
	result = 0;

	switch(command)
	{
		case COMPLETED:
			result = 1;
			break;

		case GET_SAMPLERATE:
			messages->write_message(mwindow->sample_rate);
			break;

		case GET_FRAMERATE:
			messages->write_message((int)(mwindow->frame_rate * 1000));
			break;

		case GET_FRAMESIZE:
			messages->write_message(mwindow->track_w, mwindow->track_h);
			break;

		case GET_SMP:
			messages->write_message(mwindow->preferences->smp + 1);
			break;

		case GET_USE_FLOAT:
			messages->write_message(mwindow->preferences->video_floatingpoint);
			break;

		case GET_USE_ALPHA:
			messages->write_message(mwindow->preferences->video_use_alpha);
			break;

		case GET_USE_INTERPOLATION:
			messages->write_message(mwindow->preferences->video_interpolate);
			break;

		case GET_ASPECT_RATIO:
			messages->write_message((long)mwindow->aspect_w, (long)mwindow->aspect_h);
			break;
		
		case GET_GUI_STATUS:
			send_gui_status(gui_on);
			break;

		case GET_REALTIME_BUFFERS:
			send_buffer_info();
			break;

		case READ_SAMPLES:
			read_samples();
			break;

		case WRITE_SAMPLES:
			write_samples();
			result = 2;
			break;
		
		case READ_FRAMES:
			read_frames();
			break;

		case WRITE_FRAMES:
			write_frames();
			result = 2;
			break;
		
		case CANCEL:
			result = 3;
// Need to get the COMPLETED that is returned by default when a plugin exits.
			messages->read_message_raw();
			break;
	}

	return result;
return 0;
}

int PluginServer::read_samples()
{
	long start_position;
	long total_samples;
	int result = 0;

	if(!plugin_open) return 0;

	messages->read_message(&total_samples, &start_position);
	if(error_flag)
	{
		messages->write_message(CANCEL);
		return 1;
	}
	else
	{
		for(int i = 0; i < tracks->total; i++)
		{
			result = ((ATrack*)tracks->values[i])->render(data_out.values[i], 
											0, 
											total_samples, 
											start_position);
		}

		messages->write_message(OK);
		return result;
	}
return 0;
}

int PluginServer::write_samples()
{
	if(!plugin_open) return 0;
	if(error_flag)
	{
		messages->write_message(CANCEL);
		return 1;
	}
	else
	{
		written_samples = messages->read_message();
		return 0;
	}
return 0;
}

long PluginServer::get_written_samples()
{
	if(!plugin_open) return 0;
	return written_samples;
}

int PluginServer::read_frames()
{
	long start_position;
	long total_frames;
	int result = 0;

	if(!plugin_open) return 0;
	messages->read_message(&total_frames, &start_position);
	if(error_flag)
	{
		messages->write_message(CANCEL);
		return 1;
	}
	else
	{
		for(long i = 0, j; i < tracks->total; i++)
		{
			for(j = 0; j < total_frames; j++)
			{
// Create a temporary pointer to a frame in the plugin buffer.
				VFrame *frame[1];
				long frame_vpixel = j * mwindow->track_w * mwindow->track_h;

				frame[0] = new VFrame((unsigned char*)((VPixel*)data_out.values[i]->get_data() + frame_vpixel),
							mwindow->track_w, mwindow->track_h,
							VFRAME_VPIXEL);
				result = ((VTrack*)tracks->values[i])->render(frame, 
											data_out.values[i],
											frame_vpixel * sizeof(VPixel), 
											1, 
											start_position + j, 
											1);
				delete frame[0];
			}
		}

		messages->write_message(OK);
		return result;
	}
return 0;
}

int PluginServer::write_frames()
{
	if(!plugin_open) return 0;
	if(error_flag)
	{
		messages->write_message(CANCEL);
		return 1;
	}
	else
	{
		written_frames = messages->read_message();
		return 0;
	}
return 0;
}

long PluginServer::get_written_frames()
{
	if(!plugin_open) return 0;
	return written_frames;
}






int PluginServer::load_defaults()             // loads defaults from disk file
{
	if(!plugin_open) return 0;
	messages->write_message(LOAD_DEFAULTS);
return 0;
}

int PluginServer::save_defaults()             // save defaults from disk file
{
	if(!plugin_open) return 0;
	messages->write_message(SAVE_DEFAULTS);
return 0;
}

int PluginServer::get_parameters()      // waits for plugin to finish and returns a result
{
	if(!plugin_open) return 0;
	messages->write_message(GET_PARAMETERS);
	
// handle plugin requests while getting parameters
	plugin_server_loop();
	
// get the status of the plugin
	int result = messages->read_message();
	return result;
return 0;
}





// ======================= Non-realtime plugin

int PluginServer::set_interactive()
{
	if(!plugin_open) return 0;
	messages->write_message(SET_INTERACTIVE);
return 0;
}

int PluginServer::set_range(long start, long end)
{
	if(!plugin_open) return 0;
	messages->write_message(SET_RANGE);
	messages->write_message(start, end);
return 0;
}

int PluginServer::set_track(Track *track)
{
	tracks->append(track);
return 0;
}

int PluginServer::set_error()
{
	error_flag = 1;
return 0;
}

int PluginServer::set_realtime_sched()
{
	struct sched_param params;
	params.sched_priority = 1;
	if(sched_setscheduler(fork_thread->plugin_pid, SCHED_RR, &params)) perror("sched_setscheduler");
return 0;
}

int PluginServer::send_cancel()
{
	if(!plugin_open) return 0;
	messages->write_message(CANCEL);
return 0;
}

int PluginServer::send_write_result(int result)
{
	if(!plugin_open) return 0;
	if(!result) messages->write_message(OK);
	else messages->write_message(CANCEL);
return 0;
}

int PluginServer::start_plugin() 
{
	if(!plugin_open) return 0;
	messages->write_message(START_PLUGIN); 
	return 0;
return 0;
}

int PluginServer::send_gui_status(int visible)
{
	if(!plugin_open) return 0;
	messages->write_message(visible ? GUI_ON : GUI_OFF);
	return 0;
return 0;
}

int PluginServer::set_string(char *string)
{
	if(!plugin_open) return 0;
	messages->write_message(SET_STRING);
	messages->write_message(string);
return 0;
}

int PluginServer::process_realtime(long source_len, long source_position, long fragment_len)
{
	process_realtime_start(source_len, source_position, fragment_len);
	process_realtime_end();
return 0;
}

int PluginServer::process_realtime_start(long source_len, long source_position, long fragment_len)
{
	if(!plugin_open) return 0;
	if(message_lock) message_lock->lock();
	messages->write_message(PROCESS_REALTIME);

// send information on the buffers
	messages->write_message(fragment_len, source_len, source_position);
	int i;
	for(i = 0; i < total_in_buffers; i++)
	{
		messages->write_message(offset_in_render.values[i], double_buffer_in_render.values[i]);
	}
	for(i = 0; i < total_out_buffers; i++)
	{
		messages->write_message(offset_out_render.values[i], double_buffer_out_render.values[i]);
	}
// Send information on the automation
	send_automation(source_len, source_position, fragment_len);
return 0;
}

int PluginServer::send_automation(long source_len, long source_position, long buffer_len)
{
	long position;
	int i, done = 0;
	FloatAuto *current;
	int automate = 1;
	float constant = 0;
	long buffer_position;
	long input_start;
	long input_end;
	float slope_value;
	float slope_start;
	float slope_end;
	float slope_position;
	float slope;

	if(autos)
	{
		autos->init_automation(buffer_position, 
						input_start, 
						input_end, 
						automate, 
						constant, 
						source_position,
						buffer_len,
						(Auto**)start_auto, 
						(Auto**)end_auto,
						reverse);

		if(automate)
		{
			autos->init_slope((Auto**)&current, 
					slope_start,
					slope_value,
					slope_position, 
					input_start,
					input_end, 
					(Auto**)start_auto, 
					(Auto**)end_auto,
					reverse);

			while(buffer_position < buffer_len)
			{
				autos->get_slope((Auto**)&current, 
						slope_start, 
						slope_end, 
						slope_value, 
						slope, 
						buffer_len, 
						buffer_position,
						reverse);

				messages->write_message_f(buffer_position, slope_value + slope_position * slope, slope);
				buffer_position += (long)(slope_end - slope_position);
				slope_position = slope_end;
				autos->advance_slope((Auto**)&current, 
								slope_start, 
								slope_value,
								slope_position, 
								reverse);
			}
		}
		else
		{
// Send constant if no automation but constant is changed
			if(constant != 0)
				messages->write_message_f(0, constant, (float)0);
		}
	}

	messages->write_message_f(-1, (float)0, (float)0);
return 0;
}

int PluginServer::process_realtime_end()
{
	if(!plugin_open) return 0;
	messages->read_message();       // wait for completed
	if(message_lock) message_lock->unlock();
	start_auto = end_auto = 0;
	autos = 0;
return 0;
}


// ============================= queries
int PluginServer::get_realtime()
{
	int result;
	if(!plugin_open) return 0;
	messages->write_message(GET_REALTIME);
	result = messages->read_message();
	return result;
return 0;
}

int PluginServer::get_multichannel()
{
	if(!plugin_open) return 0;
	messages->write_message(GET_MULTICHANNEL);
	return messages->read_message();
return 0;
}

int PluginServer::get_title(char *title)
{
	if(!plugin_open) return 0;
	messages->write_message(GET_TITLE);
	messages->read_message(title);
return 0;
}

int PluginServer::get_audio()
{
	if(!plugin_open) return 0;
	messages->write_message(GET_AUDIO);
	return messages->read_message();
return 0;
}

int PluginServer::get_video()
{
	if(!plugin_open) return 0;
	messages->write_message(GET_VIDEO);
	return messages->read_message();
return 0;
}

int PluginServer::get_fileio()
{
	if(!plugin_open) return 0;
	messages->write_message(GET_FILEIO);
	return messages->read_message();
return 0;
}

int PluginServer::get_samplerate()
{
	if(!plugin_open) return 0;
	if(audio)
	{
		messages->write_message(GET_SAMPLERATE);
		int result = messages->read_message();
		if(result == -1) return mwindow->sample_rate;    // plugin doesn't change sample rate
		else return result;      // plugin changes sample rate
	}
	else
	return mwindow->sample_rate;
return 0;
}


float PluginServer::get_framerate()
{
	if(!plugin_open) return 0;
	if(video)
	{
		messages->write_message(GET_FRAMERATE);
		float result = messages->read_message();
		if(result < 0) return mwindow->frame_rate;    // plugin doesn't change frame rate
		else return result/ 1000;      // plugin changes frame rate
	}
	else
	return mwindow->frame_rate;
}

int PluginServer::negotiate_buffers(long recommended_size)
{
	if(!plugin_open) return 0;
// prepare to negotiate buffers
	messages->write_message(GET_BUFFERS);
// send number of tracks and recommended size for input and output buffers
	messages->write_message(tracks->total, recommended_size);

// get actual sizes	
	messages->read_message(&in_buffer_size, &out_buffer_size);  // get desired sizes
// init buffers
	total_out_buffers = total_in_buffers = tracks->total;

// init buffers
	int word_size;
	if(video) 
		word_size = mwindow->track_w * mwindow->track_h * sizeof(VPixel);
	else
	if(audio)
		word_size = sizeof(float);

// Sizes sent back to the client are byte counts.
	for(int i = 0; i < tracks->total; i++)
	{
		data_out.append(new PluginBuffer(out_buffer_size, word_size));

		messages->write_message(data_out.values[i]->get_id(), data_out.values[i]->get_size());
	}

	for(int i = 0; i < tracks->total; i++)
	{
		data_in.append(new PluginBuffer(in_buffer_size, word_size));
		messages->write_message(data_in.values[i]->get_id(), data_in.values[i]->get_size());
	}
return 0;
}

int PluginServer::attach_input_buffer(PluginBuffer *input, long size)
{
	shared_buffers = 1;     // all buffers are shared

	data_in.append(input);

	in_buffer_size = size;
	total_in_buffers++;
	return total_in_buffers - 1;
return 0;
}

int PluginServer::attach_input_buffer(PluginBuffer **input, long double_buffers, long buffer_size, long fragment_size)
{
	shared_buffers = 1;     // all buffers are shared

//printf("PluginServer::attach_input_buffer %p %d %d %d\n", input, double_buffers, buffer_size, fragment_size);
	data_in_realtime.append(input);
	double_buffers_in.append(double_buffers);
	offset_in_render.append(0);
	double_buffer_in_render.append(0);
	realtime_in_size.append(buffer_size);
	in_buffer_size = fragment_size;

	return total_in_buffers++;
return 0;
}

int PluginServer::attach_output_buffer(PluginBuffer *output, long size)
{
	shared_buffers = 1;     // all buffers are shared

	data_out.append(output);

	total_out_buffers++;
	out_buffer_size = size;
	return total_out_buffers - 1;
return 0;
}


int PluginServer::attach_output_buffer(PluginBuffer **output, long double_buffers, long buffer_size, long fragment_size)
{
//printf("PluginServer::attach_output_buffer %p %d %d %d\n", output, double_buffers, buffer_size, fragment_size);
	shared_buffers = 1;     // all buffers are shared

	data_out_realtime.append(output);
	double_buffers_out.append(double_buffers);
	offset_out_render.append(0);
	double_buffer_out_render.append(0);
	realtime_out_size.append(buffer_size);
	out_buffer_size = fragment_size;

	return total_out_buffers++;
return 0;
}

int PluginServer::detach_buffers()
{
	data_out_realtime.remove_all();
	double_buffers_out.remove_all();
	offset_out_render.remove_all();
	double_buffer_out_render.remove_all();
	realtime_out_size.remove_all();

	data_in_realtime.remove_all();
	double_buffers_in.remove_all();
	offset_in_render.remove_all();
	double_buffer_in_render.remove_all();
	realtime_in_size.remove_all();
	
	out_buffer_size = 0;
	shared_buffers = 0;
	total_out_buffers = 0;
	in_buffer_size = 0;
	total_in_buffers = 0;
return 0;
}

int PluginServer::arm_buffer(int buffer_number, 
		long offset_in, 
		long offset_out,
		int double_buffer_in,
		int double_buffer_out)
{
	offset_in_render.values[buffer_number] = offset_in;
	offset_out_render.values[buffer_number] = offset_out;
	double_buffer_in_render.values[buffer_number] = double_buffer_in;
	double_buffer_out_render.values[buffer_number] = double_buffer_out;
return 0;
}

int PluginServer::init_realtime(int realtime_sched)
{
	if(!plugin_open) return 0;
// set for realtime priority
	if(realtime_sched) set_realtime_sched();
// prepare to negotiate buffers
	messages->write_message(START_REALTIME);
	message_lock = new Sema;
// handle configuration requests to set up rendering
	plugin_server_loop();
return 0;
}


int PluginServer::restart_realtime()
{
	messages->write_message(RESTART_REALTIME);
	send_buffer_info();
return 0;
}

int PluginServer::set_automation(FloatAutos *autos, FloatAuto **start_auto, FloatAuto **end_auto, int reverse)
{
	this->autos = autos;
	this->start_auto = start_auto;
	this->end_auto = end_auto;
	this->reverse = reverse;
return 0;
}


int PluginServer::send_buffer_info()
{
	if(!plugin_open) return 0;
// send number of tracks and size of each buffer
// assume there are as many input buffers as output buffers and 
// all buffers are the same size
	messages->write_message(message_lock->get_id());
	messages->write_message(total_in_buffers, in_buffer_size);

	int i, j;
// send input buffers
	for(i = 0; i < total_in_buffers; i++)
	{
// send number of double buffers for this channel and size of each double buffer
		messages->write_message(double_buffers_in.values[i], realtime_in_size.values[i]);
// send id of each double buffer
		for(j = 0; j < double_buffers_in.values[i]; j++)
		{
			messages->write_message(data_in_realtime.values[i][j]->get_id());
		}
	}

// send output buffers
	for(i = 0; i < total_out_buffers; i++)
	{
// send number of double buffers for this channel
		messages->write_message(double_buffers_out.values[i], realtime_out_size.values[i]);
// send id of each double buffer
		for(j = 0; j < double_buffers_out.values[i]; j++)
		{
			messages->write_message(data_out_realtime.values[i][j]->get_id());
		}
	}
return 0;
}

int PluginServer::realtime_stop()
{
	if(!plugin_open) return 0;
// detach from buffers
	message_lock->lock();
	messages->write_message(STOP_REALTIME);
	message_lock->unlock();

// delete pointers to all the buffers
// these are always shared
	data_in_realtime.remove_all();
	double_buffers_in.remove_all();
	data_out_realtime.remove_all();
	double_buffers_out.remove_all();
	offset_in_render.remove_all();
	offset_out_render.remove_all();
	double_buffer_in_render.remove_all();
	double_buffer_out_render.remove_all();

	delete message_lock;
	message_lock = 0;
	in_buffer_size = out_buffer_size = 0;
	shared_buffers = 0;
return 0;
}

char* PluginServer::save_data()
{
	if(!plugin_open) return "";
	messages->write_message(SAVE_DATA);
	return messages->read_message_raw();
}

int PluginServer::notify_load_data()
{
	if(!plugin_open) return 0;
// send the notification
	messages->write_message(LOAD_DATA);
return 0;
}


int PluginServer::load_data()
{
	if(!plugin_open) return 0;
// send the text that was previously loaded by char* get_message_buffer()
	messages->write_message_raw();
return 0;
}

int PluginServer::get_configuration_change(char *data)
{
	if(!plugin_open) return 0;
	if(message_lock) message_lock->lock();
	messages->write_message(CONFIGURE_CHANGE);
	messages->write_message(data);
	if(message_lock) message_lock->unlock();
return 0;
}
