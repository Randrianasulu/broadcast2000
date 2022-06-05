#include "errorbox.h"
#include "cdripper.h"
#include "cdripwindow.h"
#include "sizes.h"

#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	CDRipMain *plugin;
	plugin = new CDRipMain(argc, argv);
	plugin->plugin_run();
}

CDRipMain::CDRipMain(int argc, char *argv[])
 : PluginAClient(argc, argv)
{
	defaults = 0;
}

CDRipMain::~CDRipMain()
{
	if(defaults) delete defaults;
}

int CDRipMain::run_client()
{
	plugin_exit();
return 0;
}

const char* CDRipMain::plugin_title() { return "CD Ripper"; }

int CDRipMain::plugin_is_realtime() { return 0; }

int CDRipMain::plugin_is_multi_channel() { return 1; }

int CDRipMain::get_plugin_samplerate() { return 44100; }          // all CDs are 44100

long CDRipMain::get_in_buffers(long recommended_size)
{
	return 64000;
}

long CDRipMain::get_out_buffers(long recommended_size)
{
	return 64000;
}

int CDRipMain::load_defaults()
{
// set the default directory
	char directory[1024];
	sprintf(directory, "%scdripper.rc", BCASTDIR);

// load the defaults
	defaults = new Defaults(directory);
	defaults->load();

	track1 = defaults->get("TRACK1", 1);
	min1 = defaults->get("MIN1", 0);
	sec1 = defaults->get("SEC1", 0);
	track2 = defaults->get("TRACK2", 2);
	min2 = defaults->get("MIN2", 0);
	sec2 = defaults->get("SEC2", 0);
	sprintf(device, "/dev/cdrom");
	defaults->get("DEVICE", device);
	startlba = defaults->get("STARTLBA", 0);
	endlba = defaults->get("ENDLBA", 0);
return 0;
}

int CDRipMain::save_defaults()
{
	defaults->update("TRACK1", track1);
	defaults->update("MIN1", min1);
	defaults->update("SEC1", sec1);
	defaults->update("TRACK2", track2);
	defaults->update("MIN2", min2);
	defaults->update("SEC2", sec2);
	defaults->update("DEVICE", device);
	defaults->update("STARTLBA", startlba);
	defaults->update("ENDLBA", endlba);
	defaults->save();
return 0;
}

int CDRipMain::get_parameters()
{
	int result, result2;

	result = 0;
	result2 = 1;
	
	while(result2 && !result)
	{
		{
			CDRipWindow window(this);
			window.create_objects();
			result = window.run_window();
		}
		if(!result) result2 = get_toc();
	}
	
	send_completed();
	return result;
}

int CDRipMain::open_drive()
{
	if((cdrom = open(device, O_RDONLY)) < 0)
	{
		ErrorBox window;
		window.create_objects("Can't open cdrom drive.");
		window.run_window();
		return 1;
	}

	ioctl(cdrom, CDROMSTART);         // start motor
	return 0;
}

int CDRipMain::close_drive()
{
	ioctl(cdrom, CDROMSTOP);
	close(cdrom);
	return 0;
}

int CDRipMain::get_toc()
{
// test CD
	int result = 0, i, tracks;
	struct cdrom_tochdr hdr;
	struct cdrom_tocentry entry[100];
	
	result = open_drive();
	
	if(ioctl(cdrom, CDROMREADTOCHDR, &hdr) < 0)
	{
		close(cdrom);
 		ErrorBox window;
		window.create_objects("Can't get total from table of contents.");
		window.run_window();
		result = 1;
  	}
  			
  	for(i = 0; i < hdr.cdth_trk1; i++)
  	{
		entry[i].cdte_track = 1 + i;
		entry[i].cdte_format = CDROM_LBA;
		if(ioctl(cdrom, CDROMREADTOCENTRY, &entry[i]) < 0)
		{
			ioctl(cdrom, CDROMSTOP);
			close(cdrom);
 			ErrorBox window;
			window.create_objects("Can't get table of contents entry.");
			window.run_window();
			result = 1;
			break;
		}
  	}

  	entry[i].cdte_track = CDROM_LEADOUT;
  	entry[i].cdte_format = CDROM_LBA;
	if(ioctl(cdrom, CDROMREADTOCENTRY, &entry[i]) < 0)
	{
		ioctl(cdrom, CDROMSTOP);
		close(cdrom);
 		ErrorBox window;
		window.create_objects("Can't get table of contents leadout.");
		window.run_window();
		result = 1;
	}
			
			
  	tracks = hdr.cdth_trk1+1;

	if(track1 <= 0 || track1 > tracks)
	{
		ioctl(cdrom, CDROMSTOP);
		close(cdrom);
 		ErrorBox window;
		window.create_objects("Start track is out of range.");
		window.run_window();
		result = 1;
	}
	
	if(track2 < track1 || track2 <= 0 || track2 > tracks)
	{
		ioctl(cdrom, CDROMSTOP);
		close(cdrom);
 		ErrorBox window;
		window.create_objects("End track is out of range.");
		window.run_window();
		result = 1;
	}
	
	if(track1 == track2 && min2 == 0 && sec2 == 0)
	{
		ioctl(cdrom, CDROMSTOP);
		close(cdrom);
 		ErrorBox window;
		window.create_objects("End position is out of range.");
		window.run_window();
		result = 1;
	}

	startlba = endlba = 0;
	if(!result)
	{
		startlba = entry[track1 - 1].cdte_addr.lba;
		startlba += (min1 * 44100 * 4 * 60 + sec1 * 44100 * 4) / FRAMESIZE;

		endlba = entry[track2 - 1].cdte_addr.lba;
		if(track2 < tracks)
		{
			endlba += (min2 * 44100 * 4 * 60 + sec2 * 44100 * 4) / FRAMESIZE;
		}
	}

//printf("%ld %ld\n", startlba, endlba);
	close_drive();
	return result;
}

int CDRipMain::start_plugin()
{
// get data buffers
// only the output is used but load both for practice
	float **buffer_in, **buffer_out;

	buffer_in = new float*[total_in_buffers];
	buffer_out = new float*[total_out_buffers];
	
	for(int i = 0; i < total_in_buffers; i++)
		buffer_in[i] = (float*)data_in[i]->get_data();
		
	for(int i = 0; i < total_out_buffers; i++)	
		buffer_out[i] = (float*)data_out[i]->get_data();

// get CD parameters	
	int result = 0;
	
	struct cdrom_read_audio arg;
	const int FRAME = 4;    // 2 bytes 2 channels
	int previewing = 3;     // defeat bug in hardware
	long fragment_length = out_buffer_size * FRAME;
	fragment_length /= NFRAMES * FRAMESIZE;
	fragment_length *= NFRAMES * FRAMESIZE;
	long total_length = (endlba - startlba) * FRAMESIZE / fragment_length + previewing + 1;

	result = open_drive();
	
// thread out progress
	BC_ProgressBox *progress;
	
	if(interactive)
	{
		progress = new BC_ProgressBox("", "CD Ripper", total_length);
		progress->start();
	}

// get still more CD parameters
	int endofselection = 0;
	int i, j, k, l, attempts;
	long fragment_samples;
	long currentlength = 0;
	long startlba_ = startlba - fragment_length * previewing / FRAMESIZE;
	char *buffer = new char[fragment_length];
	TWO *buffer_channel;
	float *output_buffer;


	
	arg.addr_format = CDROM_LBA;
	arg.nframes = NFRAMES;

// render it
	for(arg.addr.lba = startlba_; 
		arg.addr.lba < endlba && !endofselection && !result;)
	{
		if(arg.addr.lba + fragment_length / FRAMESIZE > endlba)
		{
			fragment_length = (endlba - arg.addr.lba) / NFRAMES;
			fragment_length *= NFRAMES * FRAMESIZE;
			endofselection = 1;
		}
		
		for(i = 0; i < fragment_length; 
			i += NFRAMES * FRAMESIZE,
			arg.addr.lba += NFRAMES)
		{
			arg.buf = (unsigned char*)&buffer[i];
			for(attempts = 0; attempts < 3; attempts++)
			{
				if(!(ioctl(cdrom, CDROMREADAUDIO, &arg)))
				{
					attempts = 3;
				}
				else
				if(attempts == 2 && !previewing) printf("Can't read CD audio.\n");
			}
		}
		
		if(arg.addr.lba > startlba)
		{
// convert to floats
			fragment_samples = fragment_length / FRAME;
			for(j = 0; j < total_out_buffers; j++)
			{
				buffer_channel = (TWO*)buffer + j;
				output_buffer = buffer_out[j];
				for(k = 0, l = 0; l < fragment_samples; k += 2, l++)
				{
					output_buffer[l] = buffer_channel[k];
					output_buffer[l] /= 0x7fff;
				}
			}

			result = write_samples(fragment_samples);
		}

		currentlength++;
		if(interactive)
		{
			if(!result) result = progress->update(currentlength);
			if(progress->cancelled()) send_cancelled();
		}
	}

	if(interactive)
	{
		progress->stop_progress();
		delete progress;
	}

	delete buffer;
	close_drive();

// never recieved if error condition
	send_completed();
	
	delete [] buffer_in;
	delete [] buffer_out;
return 0;
}
