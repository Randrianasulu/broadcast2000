#include <string.h>
#include "assets.h"
#include "confirmsave.h"
#include "file.h"
#include "neworappend.h"

NewOrAppend::NewOrAppend()
{
}

NewOrAppend::~NewOrAppend()
{
}

// give the user three options if the file exists
int NewOrAppend::test_file(char *display, Asset *asset, FileHTAL *script)
{
	FILE *file;
	int result = 0;
	if(file = fopen(asset->path, "r"))
	{
		fclose(file);
		if(!script)
		{
// Don't ask user if running a script.
			ConfirmSaveWindow window(display, asset->path);
			window.create_objects();
			result = window.run_window();
		}
		else
		{
			result = 0;
		}
		return result;
	}
	else
	{
		return 0;
	}

// 	File file;
// 	Asset test_asset(asset->path);
// // open and get information if possible
// //printf("NewOrAppend::test_file 1 %s\n", asset->path);
// 	int result = file.try_to_open_file(mwindow->plugindb, &test_asset, 1, 0);
// //printf("NewOrAppend::test_file 2 %s\n", asset->path);
// 
// 	switch(result)
// 	{
// 		case 0:      // exists and format found
// 			file.close_file();
// // test format
// 			if(test_asset.format == asset->format == WAV &&
// 				asset->bits == test_asset.bits &&
// 				asset->rate == test_asset.rate &&
// 				asset->channels == test_asset.channels
// 				)
// 			{
// // only append to WAV of same bitrate
// 				NewOrAppendWindow window(display, asset, 1);
// 				window.create_objects();
// 				result = window.run_window();
// 				return result;
// 			}
// 			else
// 			{
// 				ConfirmSaveWindow window(display, asset->path);
// 				window.create_objects();
// 				result = window.run_window();
// //printf("NewOrAppend::test_file 4\n");
// 				return result;
// 			}
// 			break;
// 
// 		case 1:        // not found
// 			return 0;
// 			break;
// 		
// 		case 2:        // unknown format
// 			if(asset->format == PCM)
// 			{
// // only append to PCM
// 				NewOrAppendWindow window(display, asset, 2);
// 				window.create_objects();
// 				result = window.run_window();
// 				return result;
// 			}
// 			else
// 			{
// //printf("NewOrAppend::test_file 2\n");
// 				ConfirmSaveWindow window(display, asset->path);
// 				window.create_objects();
// 				result = window.run_window();
// //printf("NewOrAppend::test_file 3\n");
// 				return result;
// 			}
// 			break;
// 			
// 		case 3:        // HTAL file
// 			{
// //printf("NewOrAppend::test_file 3\n");
// 				ConfirmSaveWindow window(display, asset->path);
// 				window.create_objects();
// 				result = window.run_window();
// 				return result;
// 			}
// 			break;
// 	}
// //printf("NewOrAppend::test_file 5\n");
	
	return 0;
return 0;
}






NewOrAppendWindow::NewOrAppendWindow(char *display, Asset *asset, int confidence)
 : BC_Window(display, MEGREY, ICONNAME ": Overwrite", 375, 160, 375, 160)
{
	this->asset = asset;
	this->confidence = confidence;
}

NewOrAppendWindow::~NewOrAppendWindow()
{
	delete ok;
	delete cancel;
	delete append;
}

int NewOrAppendWindow::create_objects()
{
	char string[1024], filename[1024];
	FileSystem fs;
	fs.extract_name(filename, asset->path);
	
	sprintf(string, "%s exists!", filename);
	add_tool(new BC_Title(5, 5, string));
	if(confidence == 1)
	sprintf(string, "But is in the same format as your new file.");
	else
	sprintf(string, "But might be in the same format as your new file.");
	add_tool(new BC_Title(5, 25, string));
	add_tool(ok = new NewOrAppendNewButton(this));
	add_tool(append = new NewOrAppendAppendButton(this));
	add_tool(cancel = new NewOrAppendCancelButton(this));
return 0;
}

NewOrAppendNewButton::NewOrAppendNewButton(NewOrAppendWindow *nwindow)
 : BC_BigButton(30, 45, "Overwrite it")
{
	this->nwindow = nwindow;
}

int NewOrAppendNewButton::handle_event()
{
	nwindow->set_done(0);
return 0;
}

int NewOrAppendNewButton::keypress_event()
{
	if(nwindow->get_keypress() == 13) { handle_event(); return 1; }
	return 0;
return 0;
}

NewOrAppendAppendButton::NewOrAppendAppendButton(NewOrAppendWindow *nwindow)
 : BC_BigButton(30, 80, "Append to it")
{
	this->nwindow = nwindow;
}

int NewOrAppendAppendButton::handle_event()
{
	nwindow->set_done(2);
return 0;
}

int NewOrAppendAppendButton::keypress_event()
{
	if(nwindow->get_keypress() == 13) { handle_event(); return 1; }
	return 0;
return 0;
}

NewOrAppendCancelButton::NewOrAppendCancelButton(NewOrAppendWindow *nwindow)
 : BC_BigButton(30, 115, "Cancel operation")
{
	this->nwindow = nwindow;
}

int NewOrAppendCancelButton::handle_event()
{
	nwindow->set_done(1);
return 0;
}

int NewOrAppendCancelButton::keypress_event()
{
	if(nwindow->get_keypress() == ESC) { handle_event(); return 1; }
	return 0;
return 0;
}

