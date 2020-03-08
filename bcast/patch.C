#include <string.h>
#include "console.h"
#include "filehtal.h"
#include "mainwindow.h"
#include "module.h"
#include "modules.h"
#include "patch.h"
#include "patchbay.h"

Patch::Patch(MainWindow *mwindow, PatchBay *patchbay, int data_type) : ListItem<Patch>()
{
	this->mwindow = mwindow;
	this->patches = patchbay;
	this->data_type = data_type;
	record = play = automate = 1;
	draw = 0;
	title[0] = 0;
}

Patch::~Patch()
{
	if(mwindow->gui)
	{
		delete recordpatch;  
		delete playpatch;  
		delete autopatch;  
		delete drawpatch;
		delete title_text;  
	}
}

int Patch::create_objects(char *text, int pixel)
{
	int x, y;
	this->pixel = pixel;
	strcpy(title, text);
	
	if(mwindow->gui)
	{
		if(mwindow->tracks_vertical)
		{
			//x = patches->gui->w - pixel - mwindow->zoom_track;
			x = pixel + 3;
			y = 0;
		}
		else
		{
			x = 3;
			y = pixel;
		}

 		patches->gui->add_tool(recordpatch = new RecordPatch(this, x, y));
		patches->gui->add_tool(playpatch = new PlayPatch(this, x, y));
		patches->gui->add_tool(title_text = new TitlePatch(this, text, x, y));
		//patches->gui->add_tool(autotitle = new BC_Title(x + PATCH_AUTO_TITLE, y + PATCH_ROW2, "A", SMALLFONT));
		patches->gui->add_tool(autopatch = new AutoPatch(this, x, y));
		//patches->gui->add_tool(drawtitle = new BC_Title(x + PATCH_DRAW_TITLE, y + PATCH_ROW2, "D", SMALLFONT));
		patches->gui->add_tool(drawpatch = new DrawPatch(this, x, y));
	}
return 0;
}

int Patch::save(FileHTAL *htal)
{
	htal->tag.set_title("PATCH");
	htal->append_tag();

	if(play)
	{
		htal->tag.set_title("PLAY");
		htal->append_tag();
	}
	
	if(record)  
	{
		htal->tag.set_title("RECORD");
		htal->append_tag();
	}
	
	if(automate)  
	{
		htal->tag.set_title("AUTO");
		htal->append_tag();
	}
	
	if(draw)  
	{
		htal->tag.set_title("DRAW");
		htal->append_tag();
	}
	
	htal->tag.set_title("TITLE");
	htal->append_tag();
	htal->append_text(title);
	htal->tag.set_title("/TITLE");
	htal->append_tag();
	
	
	htal->tag.set_title("/PATCH");
	htal->append_tag();
	htal->append_newline();
return 0;
}

int Patch::load(FileHTAL *htal)
{
	int result = 0;	
	play = record = automate = draw = 0;    // defaults
	
	do{
		result = htal->read_tag();
		
		if(!result)
		{
			if(htal->tag.title_is("/PATCH"))
			{
				result = 1;
			}
			else
			if(htal->tag.title_is("PLAY"))
			{
				play = 1;
			}
			else
			if(htal->tag.title_is("RECORD"))
			{
				record = 1;
			}
			else
			if(htal->tag.title_is("AUTO"))
			{
				automate = 1;
			}
			else
			if(htal->tag.title_is("DRAW"))
			{
				draw = 1;
			}
			else
			if(htal->tag.title_is("TITLE"))
			{
				strcpy(title, htal->read_text());
			}
		}
	}while(!result);
	
	if(mwindow->gui)
	{
		playpatch->update(play);
		recordpatch->update(record);
		autopatch->update(automate);
		drawpatch->update(draw);
		title_text->update(title);
	}
return 0;
}

int Patch::set_pixel(int pixel)
{         // must be top of track for track zoom
	this->pixel = pixel;
	if(mwindow->gui)
	{
		if(mwindow->tracks_vertical)
		{
			pixel += 3;
			playpatch->set_x(pixel + PATCH_PLAY);
			recordpatch->set_x(pixel + PATCH_REC);
			autopatch->set_x(pixel + PATCH_AUTO);
			title_text->set_x(pixel);
			//autotitle->set_x(pixel + PATCH_AUTO_TITLE);
			drawpatch->set_x(pixel + PATCH_DRAW);
			//drawtitle->set_x(pixel + PATCH_DRAW_TITLE);
		}
		else
		{
			playpatch->set_y(pixel + PATCH_ROW2);
			recordpatch->set_y(pixel + PATCH_ROW2);
			autopatch->set_y(pixel + PATCH_ROW2);
			drawpatch->set_y(pixel + PATCH_ROW2);
			title_text->set_y(pixel + 3);
			//autotitle->set_y(pixel + PATCH_ROW2);
			//drawtitle->set_y(pixel + PATCH_ROW2);
		}
	}
return 0;
}

int Patch::set_title(char *new_title)
{
	strcpy(title, new_title);
	title_text->update(new_title);
return 0;
}

int Patch::flip_vertical()
{
	if(mwindow->gui)
	{
		if(mwindow->tracks_vertical)
		{
			playpatch->set_y(PATCH_ROW2);
			recordpatch->set_y(PATCH_ROW2);
			autopatch->set_y(PATCH_ROW2);
			drawpatch->set_y(PATCH_ROW2);
			title_text->set_y(3);
			//autotitle->set_y(PATCH_ROW2);
		}
		else
		{
			playpatch->set_x(PATCH_PLAY);
			recordpatch->set_x(PATCH_REC);
			autopatch->set_x(PATCH_AUTO);
			drawpatch->set_x(PATCH_DRAW);
			title_text->set_x(PATCH_TITLE);
			//autotitle->set_x(67);
		}
		set_pixel(pixel);
	}
return 0;
}


int Patch::pixelmovement(int distance)
{
	if(mwindow->gui)
	{
		pixel -= distance;
		set_pixel(pixel);
	}
return 0;
}

Module* Patch::get_module()    // return corresponding module from console
{
	return mwindow->console->modules->module_number(patches->number_of(this));
}

PlayPatch::PlayPatch(Patch *patch, int x, int y)
 : BC_PlayPatch(x + PATCH_PLAY, y + PATCH_ROW2, 16, 16, 1)
{ this->patch = patch; patches = patch->patches; }


int PlayPatch::handle_event()
{
// get the total selected before this event
	if(shift_down())
	{
		int total_selected = patches->plays_selected();

		if(total_selected == 0)
		{
// nothing previously selected
			patches->select_all_play();
		}
		else
		if(total_selected == 1)
		{
			if(patch->play)
			{
// this patch was previously the only one on
				patches->select_all_play();
			}
			else
			{
// another patch was previously the only one on
				patches->deselect_all_play();
				patch->play = 1;
			}
		}
		else
		if(total_selected > 1)
		{
			patches->deselect_all_play();
			patch->play = 1;
		}
		
		update(patch->play);
	}
	else
	{
		patch->play = down;
	}
	patches->button_down = 1;
	patches->reconfigure_trigger = 1;
	patches->new_status = down;
return 0;
}

int PlayPatch::button_release()
{
	return 0;
return 0;
}

int PlayPatch::cursor_moved_over()
{
	if(patches->button_down && patches->new_status != down)
	{
		update(patches->new_status);
		patch->play = down;
	}
return 0;
}

RecordPatch::RecordPatch(Patch *patch, int x, int y)
 : BC_RecordPatch(x + PATCH_REC, y + PATCH_ROW2, 16, 16, 1)
{ this->patch = patch; patches = patch->patches; }

int RecordPatch::handle_event()
{
// get the total selected before this event
	if(shift_down())
	{
		int total_selected = patches->records_selected();

		if(total_selected == 0)
		{
// nothing previously selected
			patches->select_all_record();
		}
		else
		if(total_selected == 1)
		{
			if(patch->record)
			{
// this patch was previously the only one on
				patches->select_all_record();
			}
			else
			{
// another patch was previously the only one on
				patches->deselect_all_record();
				patch->record = 1;
			}
		}
		else
		if(total_selected > 1)
		{
			patches->deselect_all_record();
			patch->record = 1;
		}
		
		update(patch->record);
	}
	else
	{
		patch->record = down;
	}
	patches->button_down = 1;
	patches->new_status = down;
return 0;
}

int RecordPatch::button_release()
{
	//if(patches->button_down)
	//{
	//	patches->button_down = 0;
// restart the playback
		//patches->mwindow->start_reconfigure(1);
		//patches->mwindow->stop_reconfigure(1);
	//}
return 0;
}

int RecordPatch::cursor_moved_over()
{
	if(patches->button_down && patches->new_status != down) 
	{
		update(patches->new_status);
		patch->record = down;
	}
return 0;
}

TitlePatch::TitlePatch(Patch *patch, char *text, int x, int y)
 : BC_TextBox(x, y + PATCH_TITLE, 124, text, 0)
{ 
	this->patch = patch; 
	patches = patch->patches; 
	module = 0; 
}

int TitlePatch::handle_event()
{
	if(!module) module = patch->get_module();
	module->set_title(get_text());
	strcpy(patch->title, get_text());
return 0;
}

AutoPatch::AutoPatch(Patch *patch, int x, int y)
 : BC_CheckBox(x + PATCH_AUTO, y + PATCH_ROW2, 16, 16, 1, 0, 'A')
{ this->patch = patch; this->patches = patch->patches; }

int AutoPatch::handle_event()
{
// get the total selected before this event
	if(shift_down())
	{
		int total_selected = patches->autos_selected();

		if(total_selected == 0)
		{
// nothing previously selected
			patches->select_all_auto();
		}
		else
		if(total_selected == 1)
		{
			if(patch->automate)
			{
// this patch was previously the only one on
				patches->select_all_auto();
			}
			else
			{
// another patch was previously the only one on
				patches->deselect_all_auto();
				patch->automate = 1;
			}
		}
		else
		if(total_selected > 1)
		{
			patches->deselect_all_auto();
			patch->automate = 1;
		}
		
		update(patch->automate);
	}
	else
	{
		patch->automate = down;
	}
	patches->button_down = 1;
	patches->new_status = down;
return 0;
}

int AutoPatch::button_release()
{
	//patches->button_down = 0;
return 0;
}

int AutoPatch::cursor_moved_over()
{
	if(patches->button_down && patches->new_status != down)
	{
		update(patches->new_status);
		patch->automate = down;
	}
return 0;
}

DrawPatch::DrawPatch(Patch *patch, int x, int y)
 : BC_CheckBox(x + PATCH_DRAW, y + PATCH_ROW2, 16, 16, patch->draw, 0, 'D')
{ this->patch = patch; this->patches = patch->patches; }

int DrawPatch::handle_event()
{
// get the total selected before this event
	if(shift_down())
	{
		int total_selected = patches->draws_selected();

		if(total_selected == 0)
		{
// nothing previously selected
			patches->select_all_draw();
		}
		else
		if(total_selected == 1)
		{
			if(patch->draw)
			{
// this patch was previously the only one on
				patches->select_all_draw();
			}
			else
			{
// another patch was previously the only one on
				patches->deselect_all_draw();
				patch->draw = 1;
			}
		}
		else
		if(total_selected > 1)
		{
			patches->deselect_all_draw();
			patch->draw = 1;
		}
		
		update(patch->draw);
	}
	else
	{
		patch->draw = get_value();
	}
	patches->button_down = 1;
	patches->new_status = get_value();
return 0;
}

int DrawPatch::cursor_moved_over()
{
	if(patches->button_down && patches->new_status != get_value())
	{
		update(patches->new_status);
		patch->draw = get_value();
	}
return 0;
}
