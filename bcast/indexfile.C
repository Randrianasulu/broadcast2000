#include <string.h>
#include "assets.h"
#include "filehtal.h"
#include "filesystem.h"
#include "errorbox.h"
#include "file.h"
#include "indexfile.h"
#include "indexthread.h"
#include "mainwindow.h"
#include "mainwindowgui.h"
#include "preferences.h"
#include "progressbox.h"
#include "tracks.h"



IndexFile::IndexFile(MainWindow *mwindow)
{
	this->mwindow = mwindow;
	file = 0;
	interrupt_flag = 0;
}

IndexFile::IndexFile(MainWindow *mwindow, Asset *asset)
{
	file = 0;
	this->mwindow = mwindow;
	this->asset = asset;
	interrupt_flag = 0;
}

IndexFile::~IndexFile()
{
}

int IndexFile::open_index(MainWindow *mwindow, Asset *asset)
{
// use buffer if being built
	this->asset = asset;
	this->mwindow = mwindow;
	int result = 0;

	if(asset->index_status == 2)
	{
// use buffer
		result = 0;
	}
	else
	if(asset->index_status == 3)
	{
// source too small
		result = 1;
	}
	else
	if(!(result = open_file()))
	{
// opened successfully
		read_info();
		asset->index_status = 0;
		result = 0;
	}
	else
	{
// file too old or index file not found
		if(result == 1)
		{
// determine if index too small or no index
			File source;
			result = open_source(&source);

			if(!result)
			{
				result = get_required_scale(&source);
				source.close_file();

				if(!result)
				{
// index too small to build
					asset->index_status = 3;
				}
				source.close_file();
			}
		}
	}

	return result;
return 0;
}


int IndexFile::open_file()
{
	int result = 0;
	get_index_filename(index_filename, asset->path);

	if(file = fopen(index_filename, "rb")) 
	{
// Index file already exists.  Need its last size without changing the status.
		Asset test_asset;
		test_asset = *asset;
		read_info(&test_asset);

		FileSystem fs;
		if(fs.get_date(index_filename) < fs.get_date(test_asset.path))
		{
// index older than source
			result = 2;
			fclose(file);
		}
		else
		if(fs.get_size(asset->path) != test_asset.index_bytes)
		{
// source file is a different size than index source file
			result = 2;
			fclose(file);	
		}
		else
		{
			fseek(file, 0, SEEK_END);
			file_length = ftell(file);
			fseek(file, 0, SEEK_SET);
			result = 0;
		}
	}
	else
	{
// doesn't exist
		//printf("IndexFile::open_index() couldn't open %s\n", index_filename);
		result = 1;
	}

	return result;
return 0;
}

int IndexFile::open_source(File *source)
{
	if(source->try_to_open_file(mwindow->plugindb, asset, 1, 0))
	{
		//printf("IndexFile::open_source() Couldn't open %s.\n", asset->path);
		return 1;
	}
	else
	{
		FileSystem fs;
		asset->index_bytes = fs.get_size(asset->path);
		return 0;
	}
return 0;
}

long IndexFile::get_required_scale(File *source)
{
	long result = 0;
	long length_source = source->get_length();  // total length of input file

// get scale of index file
// change the constant depending on how tolerable the drawing rate is
	if(length_source > mwindow->preferences->index_size * 1)
	{
		for(result = 1; 
			length_source * asset->channels / result + 1 > mwindow->preferences->index_size / sizeof(float); 
			result *= 2)
			;

		result *= 2;  // each sample in the source takes two samples in the index
	}
	else
	{
// too small to build an index for
		result = 0;
	}
	
	return result;
}

int IndexFile::get_index_filename(char *index_filename, char *input_filename)
{
	FileSystem fs;
	fs.extract_name(source_filename, input_filename);
	fs.join_names(index_filename, mwindow->preferences->index_directory, source_filename);
	strcat(index_filename, ".idx");
return 0;
}

int IndexFile::interrupt_index()
{
	interrupt_flag = 1;
return 0;
}

int IndexFile::create_index(MainWindow *mwindow, Asset *asset, ProgressBox *progress)
{
	int result = 0;
	IndexThread *index_thread;

	this->mwindow = mwindow;
	this->asset = asset;
	interrupt_flag = 0;

// open the source file
	File source;
	if(open_source(&source)) return 1;

	asset->index_zoom = get_required_scale(&source);

	if(asset->index_zoom == 0)
	{
// too small to build an index for
		source.close_file();
		asset->index_status = 3;
		mwindow->gui->lock_window();
		redraw_edits(1);
		mwindow->gui->unlock_window();
		return 1;
	}

	long length_source = source.get_length();  // total length of input file

// get amount to read at a time in floats
	long buffersize = 65536;
	char string[1024];
	get_index_filename(index_filename, asset->path);
	sprintf(string, "Creating %s.", index_filename);

	progress->update_title(string);
	progress->update_length(length_source);

// thread out index thread
	index_thread = new IndexThread(mwindow, asset, index_filename, buffersize, length_source);
	index_thread->start_build();

	long position = 0;            // current sample in source file
	long fragment_size = buffersize;
	int current_buffer = 0;

// pass through file once
	while(position < length_source && !result)
	{
		if(length_source - position < fragment_size && fragment_size == buffersize) fragment_size = length_source - position;

		index_thread->input_lock[current_buffer].lock();
		index_thread->input_len[current_buffer] = fragment_size;

		if(progress->update(position) || 
			index_thread->interrupt_flag || 
			interrupt_flag)
		{
			result = 3;    // user cancelled
		}

		for(int channel = 0; !result && channel < asset->channels; channel++)
		{
			source.set_position(position);
			source.set_channel(channel);

// couldn't read
			if(source.read_samples(index_thread->shared_buffer_in[current_buffer][channel], 
				0, 
				fragment_size)) result = 1;
		}

		if(!result)
		{
			index_thread->output_lock[current_buffer].unlock();
			current_buffer++;
			if(current_buffer >= TOTAL_BUFFERS) current_buffer = 0;
			position += fragment_size;
		}
		else
		{
			index_thread->input_lock[current_buffer].unlock();
		}
	}

// end thread cleanly
	index_thread->input_lock[current_buffer].lock();
	index_thread->last_buffer[current_buffer] = 1;
	index_thread->output_lock[current_buffer].unlock();

	index_thread->stop_build();

	source.close_file();

	delete index_thread;
	return 0;
return 0;
}


int IndexFile::redraw_edits(int flash)
{
	return mwindow->tracks->set_index_file(flash, asset);
return 0;
}



int IndexFile::draw_index(BC_Canvas *canvas, 
				int pixel, 
				int center_pixel, 
				int zoom_track, 
				int zoom_sample, 
				int zoomy, 
				long startsource, 
				long endsource, 
				int sourcechan,
				int vertical)
{
// check against index_end when being built
	if(asset->index_zoom == 0)
	{
		printf("IndexFile::draw_index index has 0 zoom\n");
		return 0;
	}

	if(asset->index_status == 2)
	{
		if(endsource > asset->index_end) endsource = asset->index_end;
	}

	long length = endsource - startsource; // samples in segment to draw
	long lengthindex = length / asset->index_zoom * 2; // length index to read in floats
	long startindex = startsource / asset->index_zoom * 2;  // start of data in floats
	long length_read;   // Actual length read from file in bytes
	long startfile, lengthfile;    // Start and length of fragment to read from file in bytes.
	float *buffer;
	int i;
	int miny = center_pixel - zoom_track / 2;
	int maxy = center_pixel + zoom_track / 2;
	int y1, y2;

	zoom_sample /= asset->index_zoom;      // get zoom_sample relative to index zoomx

// test channel number
	if(sourcechan > asset->channels) return 1;

// get channel offset
	startindex += asset->index_offsets[sourcechan];



	if(asset->index_status == 2)
	{
// index is in RAM, being built
		buffer = &(asset->index_buffer[startindex]);
	}
	else
	{
// index is stored in a file
		buffer = new float[lengthindex + 1];
		startfile = asset->index_start + startindex * sizeof(float);
		lengthfile = lengthindex * sizeof(float);
		length_read = 0;

		if(startfile < file_length)
		{
			fseek(file, startfile, SEEK_SET);
			
			length_read = lengthfile;
			if(startfile + length_read > file_length)
				length_read = file_length - startfile;

			fread(buffer, length_read + sizeof(float), 1, file);
		}

		if(length_read < lengthfile)
			for(i = length_read / sizeof(float); i < lengthfile / sizeof(float); i++)
				buffer[i] = 0;
	}

	
	long highpoint = 0;
	long lowpoint = 1;
	float highsample = buffer[0];
	float lowsample = buffer[1];    // first sample
	long frame_position = 1;

	canvas->set_color(GREEN);

	for(long lo = 3, hi = 2; lo < lengthindex; lo += 2, hi += 2)
	{
		if(frame_position == zoom_sample)
		{    // draw column and reset
			if(vertical)
			{	
				y1 = (int)(center_pixel + highsample * zoomy);
				y2 = (int)(center_pixel + lowsample * zoomy);
//				if(y1 >= miny && y2 <= maxy)
					canvas->draw_line(y1, pixel, y2, pixel);
			}
			else
			{
				y1 = (int)(center_pixel - highsample * zoomy);
				y2 = (int)(center_pixel - lowsample * zoomy);
//				if(y1 >= miny && y2 <= maxy)
					canvas->draw_line(pixel, y1, pixel, y2);
			}

			frame_position = 0;
			pixel++;
		}

		if(frame_position > 0)
		{     // lowsample and highsample are set
			if(buffer[lo] < lowsample) lowsample = buffer[lo];
			if(buffer[hi] > highsample) highsample = buffer[hi];
		}
		else
		{     // first frame_position should initialize lowsample and highsample
			if(buffer[hi] < lowsample)
			{        // all samples are below lowsample
				highsample = lowsample;
				lowsample = buffer[lo];
			}
			else if(buffer[lo] > highsample) 
			{        // all samples are above highsample
				lowsample = highsample;
				highsample = buffer[hi];
			}
			else
			{        // lowsample and highsample are in between current sample
				lowsample = buffer[lo];
				highsample = buffer[hi];
			}
		}
		frame_position++;
	}


// draw last column
	if(vertical)
	{
		y1 = (int)(center_pixel + highsample * zoomy);
		y2 = (int)(center_pixel + lowsample * zoomy);
//		if(y1 >= miny && y2 <= maxy)
			canvas->draw_line(y1, pixel, y2, pixel);
	}
	else
	{
		y1 = (int)(center_pixel - highsample * zoomy);
		y2 = (int)(center_pixel - lowsample * zoomy);
//		if(y1 >= miny && y2 <= maxy)
			canvas->draw_line(pixel, y1, pixel, y2);
	}

	if(asset->index_status == 0)
	{
		delete buffer;
	}

	return 0;
return 0;
}

int IndexFile::close_index()
{
	if(file)
	{
		fclose(file);
		file = 0;
	}
return 0;
}

int IndexFile::remove_index()
{
	if(asset->index_status == 0 || asset->index_status == 1)
	{
		close_index();
		remove(index_filename);
	}
return 0;
}

int IndexFile::read_info(Asset *test_asset)
{
	if(!test_asset) test_asset = asset;
	if(test_asset->index_status == 1)
	{
// read start of index data
		fread((char*)&(test_asset->index_start), sizeof(long), 1, file);

// read test_asset info from index
		char *data;
		data = new char[test_asset->index_start];

		fread(data, test_asset->index_start - sizeof(long), 1, file);
		data[test_asset->index_start - sizeof(long)] = 0;
		FileHTAL htal;
		htal.read_from_string(data);
		test_asset->read(mwindow, &htal);
		delete [] data;
//		test_asset->index_status = 0;
	}
return 0;
}
