#include <string.h>
#include "assetedit.h"
#include "assetmanager.h"
#include "assets.h"
#include "cache.h"
#include "file.h"
#include "indexfile.h"
#include "mainwindow.h"
#include "progressbox.h"


AssetEdit::AssetEdit(AssetManagerThread *thread)
 : Thread()
{
	this->thread = thread;
	this->asset = 0;
}

AssetEdit::~AssetEdit()
{
}


int AssetEdit::set_asset(Asset *asset)
{
	this->asset = asset;
return 0;
}

void AssetEdit::run()
{
	if(asset)
	{
		Asset new_asset(asset->path);
		new_asset = *asset;
		int result;

		{
			AssetEditWindow window(thread->mwindow, &new_asset);
			window.create_objects();
			result = window.run_window();
		}

		if(!result)
		{
			if(*asset != new_asset)
			{
				thread->window->disable_window();
				thread->mwindow->cache->delete_entry(asset);
// Copy information to new asset
				if(new_asset.frame_rate < 1) new_asset.frame_rate = 1;
				*asset = new_asset;

				if(asset->audio_data)
				{
					IndexFile index(thread->mwindow);
					ProgressBox progress("", "Building Indexes...", 1, 1);
					progress.start();
					index.create_index(thread->mwindow, asset, &progress);
					progress.stop_progress();
				}
				thread->window->enable_window();
			}
		}
	}
}

AssetEditWindow::AssetEditWindow(MainWindow *mwindow, Asset *asset)
 : BC_Window(ICONNAME ": Asset Info", 415, 350, 415, 350)
{
	this->asset = asset;
	if(asset->format == PCM)
		allow_edits = 1;
	else
		allow_edits = 0;
	this->mwindow = mwindow;
}

AssetEditWindow::~AssetEditWindow()
{
}

int AssetEditWindow::create_objects()
{
	int y = 10;
	char string[1024];
	int vmargin;
	int hmargin1 = 180, hmargin2 = 290;
	FileSystem fs;

	if(allow_edits) vmargin = 30;
	else
		vmargin = 20;

	add_tool(path_text = new AssetEditPathText(this, y));
	add_tool(path_button = new AssetEditPath(this, y, path_text, 
		asset->path, 
		ICONNAME ": Asset path", "Select a file for this asset:"));
	y += 30;

	add_tool(new BC_Title(10, y, "File format:"));
	File file;
	add_tool(new BC_Title(95, y, file.formattostr(mwindow->plugindb, asset->format), MEDIUMFONT, YELLOW));

	add_tool(new BC_Title(200, y, "Bytes:"));
	sprintf(string, "%ld", fs.get_size(asset->path));
// Do commas
	int len = strlen(string);
	int commas = (len - 1) / 3;
	for(int i = len + commas, j = len, k; j >= 0 && i >= 0; i--, j--)
	{
		k = (len - j - 1) / 3;
		if(k * 3 == len - j - 1 && j != len - 1 && string[j] != 0)
		{
			string[i--] = ',';
		}

		string[i] = string[j];
	}

	add_tool(new BC_Title(260, y, string, MEDIUMFONT, YELLOW));
	y += 20;

	if(asset->audio_data)
	{
		add_tool(new BC_Title(10, y, "Audio:", LARGEFONT, RED));

		y += 40;
		add_tool(new BC_Title(10, y, "Channels:"));
		sprintf(string, "%d", asset->channels);

		if(allow_edits)
			add_tool(new AssetEditChannels(this, string, y));
		else
			add_tool(new BC_Title(90, y, string, MEDIUMFONT, YELLOW));

		add_tool(new BC_Title(hmargin1, y, "Sample rate:"));
		sprintf(string, "%d", asset->rate);

		if(allow_edits)
			add_tool(new AssetEditRate(this, string, hmargin2, y));
		else
			add_tool(new BC_Title(hmargin2, y, string, MEDIUMFONT, YELLOW));

		y += vmargin;

		add_tool(new BC_Title(10, y, "Bits:"));
		if(allow_edits)
			add_tool(new AssetEditBits(this, file.bitstostr(asset->bits), y));
		else
			add_tool(new BC_Title(90, y, file.bitstostr(asset->bits), MEDIUMFONT, YELLOW));


		add_tool(new BC_Title(hmargin1, y, "Header length:"));
		sprintf(string, "%d", asset->header);

		if(allow_edits)
			add_tool(new AssetEditHeader(this, string, hmargin2, y));
		else
			add_tool(new BC_Title(hmargin2, y, string, MEDIUMFONT, YELLOW));

		y += vmargin;

		add_tool(new BC_Title(10, y, "Byte order:"));

		if(allow_edits)
		{
			add_tool(new BC_Title(10, y + 20, "Lo-Hi:", 1));
			add_tool(lohi = new AssetEditByteOrderLOHI(this, asset->byte_order, y + 20));
			add_tool(new BC_Title(80, y + 20, "Hi-Lo:", 1));
			add_tool(hilo = new AssetEditByteOrderHILO(this, asset->byte_order ^ 1, y + 20));
		}
		else
		{
			if(asset->byte_order)
				add_tool(new BC_Title(100, y, "Lo-Hi", MEDIUMFONT, YELLOW));
			else
				add_tool(new BC_Title(100, y, "Hi-Lo", MEDIUMFONT, YELLOW));
		}


		if(allow_edits)
		{
//			add_tool(new BC_Title(hmargin1, y, "Values are signed:"));
			add_tool(new AssetEditSigned(this, asset->signed_, hmargin1, y));
		}
		else
		{
			if(!asset->signed_ && asset->bits == 8)
				add_tool(new BC_Title(hmargin1, y, "Values are unsigned"));
			else
				add_tool(new BC_Title(hmargin1, y, "Values are signed"));
		}

		y += 30;
	}

	if(asset->video_data)
	{
		add_tool(new BC_Title(10, y, "Video:", LARGEFONT, RED));
		y += 40;
		if(asset->format == MOV)
		{
			add_tool(new BC_Title(10, y, "Compression:"));
			sprintf(string, "%c%c%c%c", asset->compression[0], asset->compression[1], asset->compression[2], asset->compression[3]);
			add_tool(new BC_Title(120, y, string, MEDIUMFONT, YELLOW));
			y += vmargin;
		}

		add_tool(new BC_Title(10, y, "Frame rate:"));
		sprintf(string, "%.2f", asset->frame_rate);
		add_tool(new AssetEditFRate(this, string, 100, y));
//		add_tool(new BC_Title(100, y, string, MEDIUMFONT, YELLOW));
		
		y += 30;
		add_tool(new BC_Title(10, y, "Width:"));
		sprintf(string, "%d", asset->width);
		add_tool(new BC_Title(100, y, string, MEDIUMFONT, YELLOW));
		
		y += vmargin;
		add_tool(new BC_Title(10, y, "Height:"));
		sprintf(string, "%d", asset->height);
		add_tool(new BC_Title(100, y, string, MEDIUMFONT, YELLOW));
		y += 30;
	}

	add_tool(new AssetEditOkButton(this, get_h() - 40));
	add_tool(new AssetEditCancelButton(this, get_h() - 40));
return 0;
}


AssetEditOkButton::AssetEditOkButton(AssetEditWindow *fwindow, int y)
 : BC_BigButton(30, y, "OK")
{
	this->fwindow = fwindow;
}

int AssetEditOkButton::handle_event()
{
	fwindow->set_done(0);
return 0;
}

int AssetEditOkButton::keypress_event()
{
	if(fwindow->get_keypress() == 13) { handle_event(); return 1; }
	return 0;
return 0;
}

AssetEditCancelButton::AssetEditCancelButton(AssetEditWindow *fwindow, int y)
 : BC_BigButton(250, y, "Cancel")
{
	this->fwindow = fwindow;
}

int AssetEditCancelButton::handle_event()
{
	fwindow->set_done(1);
return 0;
}

int AssetEditCancelButton::keypress_event()
{
	if(fwindow->get_keypress() == ESC) { handle_event(); return 1; }
	return 0;
return 0;
}

AssetEditChannels::AssetEditChannels(AssetEditWindow *fwindow, const char *text, int y)
 : BC_TextBox(90, y, 50, text)
{
	this->fwindow = fwindow;
}

int AssetEditChannels::handle_event()
{
	fwindow->asset->channels = atol(get_text());
return 0;
}

AssetEditRate::AssetEditRate(AssetEditWindow *fwindow, const char *text, int x, int y)
 : BC_TextBox(x, y, 100, text)
{
	this->fwindow = fwindow;
}

int AssetEditRate::handle_event()
{
	fwindow->asset->rate = atol(get_text());
return 0;
}

AssetEditFRate::AssetEditFRate(AssetEditWindow *fwindow, const char *text, int x, int y)
 : BC_TextBox(x, y, 100, text)
{
	this->fwindow = fwindow;
}

int AssetEditFRate::handle_event()
{
	fwindow->asset->frame_rate = atof(get_text());
return 0;
}

AssetEditBits::AssetEditBits(AssetEditWindow *fwindow, const char *text, int y)
 : BitsPopup(90, y, fwindow->asset)
{
	this->fwindow = fwindow;
}

int AssetEditBits::handle_event()
{
	File file;
	fwindow->asset->bits = get_bits();
return 0;
}

AssetEditHeader::AssetEditHeader(AssetEditWindow *fwindow, const char *text, int x, int y)
 : BC_TextBox(x, y, 100, text)
{
	this->fwindow = fwindow;
}

int AssetEditHeader::handle_event()
{
	fwindow->asset->header = atol(get_text());
return 0;
}

AssetEditByteOrderLOHI::AssetEditByteOrderLOHI(AssetEditWindow *fwindow, int value, int y)
 : BC_Radial(50, y, 17, 17, value)
{
	this->fwindow = fwindow;
}

int AssetEditByteOrderLOHI::handle_event()
{
	fwindow->asset->byte_order = down;
	fwindow->hilo->update(down ^ 1);
return 0;
}

AssetEditByteOrderHILO::AssetEditByteOrderHILO(AssetEditWindow *fwindow, int value, int y)
 : BC_Radial(120, y, 17, 17, value)
{
	this->fwindow = fwindow;
}

int AssetEditByteOrderHILO::handle_event()
{
	fwindow->asset->byte_order = down ^ 1;
	fwindow->lohi->update(down ^ 1);
return 0;
}

AssetEditSigned::AssetEditSigned(AssetEditWindow *fwindow, int value, int x, int y)
 : BC_CheckBox(x, y, 17, 17, value, "Values are signed")
{
	this->fwindow = fwindow;
}

int AssetEditSigned::handle_event()
{
	fwindow->asset->signed_ = down;
return 0;
}







AssetEditPathText::AssetEditPathText(AssetEditWindow *fwindow, int y)
 : BC_TextBox(5, y, 300, fwindow->asset->path) { this->fwindow = fwindow; }
AssetEditPathText::~AssetEditPathText() {}
int AssetEditPathText::handle_event() 
{
	strcpy(fwindow->asset->path, get_text());
return 0;
}

AssetEditPath::AssetEditPath(AssetEditWindow *fwindow, int y, BC_TextBox *textbox, const char *text, const char *window_title, const char *window_caption)
 : BrowseButton(310, y, textbox, text, window_title, window_caption, 0) 
{ this->fwindow = fwindow; }
AssetEditPath::~AssetEditPath() {}






AssetEditFormat::AssetEditFormat(AssetEditWindow *fwindow, const char* default_, int y)
 : FormatPopup(fwindow->mwindow->plugindb, 90, y, default_)
{ this->fwindow = fwindow; }
AssetEditFormat::~AssetEditFormat() {}
int AssetEditFormat::handle_event()
{
	File file;
	fwindow->asset->format = file.strtoformat(fwindow->mwindow->plugindb, get_text());
return 0;
}

