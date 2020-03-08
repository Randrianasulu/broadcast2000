#ifndef INDEXFILE_H
#define INDEXFILE_H

#include "assets.inc"
#include "bcbase.h"
#include "indexthread.inc"
#include "mainwindow.inc"
#include "progressbox.inc"
#include "tracks.inc"
#include "file.inc"

class IndexFile
{
public:
	IndexFile(MainWindow *mwindow);
	IndexFile(MainWindow *mwindow, Asset *asset);
	~IndexFile();

	int open_index(MainWindow *mwindow, Asset *asset);
	int create_index(MainWindow *mwindow, Asset *asset, ProgressBox *progress);
	int interrupt_index();
	int get_index_filename(char *index_filename, char *input_filename);
	int redraw_edits(int flash = 1);
	int draw_index(BC_Canvas *canvas, 
				int pixel, 
				int center_pixel, 
				int zoom_track, 
				int zoom_sample, 
				int zoomy,
				long startsource, 
				long endsource, 
				int sourcechan,
				int vertical);
	int close_index();
	int remove_index();
	int read_info(Asset *test_asset = 0);
	int write_info();

	MainWindow *mwindow;
	char index_filename[1024], source_filename[1024];
	Asset *asset;

private:
	int open_file();
	int open_source(File *source);
	long get_required_scale(File *source);
	FILE *file;
	long file_length;   // Length of index file in bytes
	int interrupt_flag;    // Flag set when index building is interrupted
};

#endif
