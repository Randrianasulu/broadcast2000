#include "bcfilebox.h"
#include "bcsubwindow.h"
#include "filesystem.h"
#include <string.h>

BC_FileBox::BC_FileBox(char *display, char *init_directory, char *title, char *caption, int show_all_files, int want_directory)
 : BC_Window(display, MEGREY, title, 390, 405, 0, 0)
{
	dir = new FileSystem;
// defeat non existant directories
	if(init_directory[0] == 0) sprintf(init_directory, "~");

	dir->complete_path(init_directory);
	dir->extract_dir(directory, init_directory);
	dir->extract_name(filename, init_directory);
	strcpy(this->caption, caption);
	if(show_all_files) dir->set_show_all();
	if(want_directory) dir->set_want_directory();
	this->want_directory = want_directory;
	dir->update(directory);
}

BC_FileBox::~BC_FileBox()
{
	delete_tables();
	delete dir;
	delete listbox;
	delete textbox;
	delete directory_title;
	delete ok;
	delete cancel;
	if(want_directory) delete usethis;
}

int BC_FileBox::create_objects()
{
	create_tables();
	add_tool(listbox = new BC_FileBoxListBox(this));
	add_tool(new BC_Title(10, 5, caption));
	add_tool(ok = new BC_FileBoxOK(this));
	add_tool(cancel = new BC_FileBoxCancel(this));

	if(want_directory)
	{
		add_tool(textbox = new BC_FileBoxTextBox(this, directory));
		add_tool(usethis = new BC_FileBoxUseThis(this));
	}
	else
		add_tool(textbox = new BC_FileBoxTextBox(this, filename));

	add_tool(directory_title = new BC_FileBoxDirectory(this, directory));
	listbox->activate();
return 0;
}


int BC_FileBox::create_tables()
{
	delete_tables();
	column_titles[0] = "Name";
	column_titles[1] = "Size";

	column_width[0] = 250;
	column_width[1] = 100;

	int i;
	char string[1024];
	BC_ListBoxItem *new_item;
	for(i = 0; i < dir->dir_list.total; i++)
	{
		new_item = new BC_ListBoxItem(dir->dir_list.values[i]->name, 
					dir->dir_list.values[i]->is_dir ? BLUE : BLACK);
		list_column[0].append(new_item);

		new_item = new BC_ListBoxItem;
		list_column[1].append(new_item);
		if(!dir->dir_list.values[i]->is_dir)
		{
			sprintf(string, "%lld", dir->dir_list.values[i]->size);
			new_item->set_text(string);
			new_item->color = BLACK;
		}
		else
		{
			new_item->text = 0;
			new_item->color = BLUE;
		}
	}
return 0;
}

int BC_FileBox::delete_tables()
{
	int i;
	for(i = 0; i < list_column[0].total; i++) delete list_column[0].values[i];
	for(i = 0; i < list_column[1].total; i++) delete list_column[1].values[i];
	list_column[0].remove_all();
	list_column[1].remove_all();
return 0;
}

int BC_FileBox::ok_event()
{
	set_done(0);
return 0;
}


int BC_FileBox::cancel_event()
{
	set_done(1);
return 0;
}

char* BC_FileBox::get_directory()
{
	return directory;
}

char* BC_FileBox::get_filename()
{
	return filename;
}

int BC_FileBox::set_show_all()
{
	dir->set_show_all();
	dir->update(directory);
return 0;
}

int BC_FileBox::submit_file(char *text)
{
	if(!text[0]) return 1;   // blank

	if(!dir->is_dir(text))
	{            // is a directory, change directories
		dir->change_dir(text);     // change to it
		create_tables();
		listbox->set_contents(list_column, column_titles, BC_FILEBOX_TOTAL_COLUMNS, 0, 0);
		directory_title->update(dir->get_current_dir());

		if(strlen(dir->get_current_dir())) strcpy(directory, dir->get_current_dir());
		if(want_directory)
			textbox->update(dir->get_current_dir());
		else
			textbox->update("");

		listbox->reset_query();
		return 1;
	}
	else
	{            // not a directory, quit now
		dir->extract_dir(directory, text);     // directory is saved in directory for default updating
		dir->complete_path(text);
		strcpy(filename, text);          // save complete path in filename
		return 0;
	}
return 0;
}

BC_FileBoxOK::BC_FileBoxOK(BC_FileBox *filebox)
 : BC_BigButton(10, 370, "OK")
{ this->filebox = filebox; }

int BC_FileBoxOK::handle_event()
{
	int result = 0;
	if(strlen(filebox->textbox->get_text()) && 
		!filebox->submit_file(filebox->textbox->get_text()))
	{
		result = 1;
	}
	
	if(result && !filebox->want_directory)
	{
		filebox->ok_event();
		filebox->set_done(0);
	}
return 0;
}

int BC_FileBoxOK::keypress_event()
{
	if(get_keypress() == 13) { handle_event(); return 1; }
	return 0;
return 0;
}








BC_FileBoxUseThis::BC_FileBoxUseThis(BC_FileBox *filebox)
 : BC_BigButton(100, 370, "Use This")
{ this->filebox = filebox; }

int BC_FileBoxUseThis::handle_event()
{
	if(strlen(filebox->textbox->get_text()) &&
		filebox->submit_file(filebox->textbox->get_text()))
	{
		filebox->ok_event();
		set_done(0);
	}
return 0;
}






BC_FileBoxCancel::BC_FileBoxCancel(BC_FileBox *filebox) 
 : BC_BigButton(200, 370, "Cancel")
{ this->filebox = filebox; }

int BC_FileBoxCancel::handle_event()
{
// get it saved for default updates
	if(strlen(filebox->textbox->get_text())) filebox->submit_file(filebox->textbox->get_text());
	filebox->cancel_event();
	set_done(1);
return 0;
}

int BC_FileBoxCancel::keypress_event()
{
	if(top_level->get_keypress() == ESC) { handle_event(); return 1; }
	return 0;
return 0;
}









BC_FileBoxTextBox::BC_FileBoxTextBox(BC_FileBox *filebox, char *text)
 : BC_TextBox(10, 340, 300, text)
{ this->filebox = filebox; }

int BC_FileBoxTextBox::handle_event()
{
return 0;
}




BC_FileBoxDirectory::BC_FileBoxDirectory(BC_FileBox *filebox, char *text)
 : BC_Title(10, 320, filebox->get_directory())
{
}






BC_FileBoxListBox::BC_FileBoxListBox(BC_FileBox *filebox)
 : BC_ListBox(10, 30, 350, 270, 
 			filebox->list_column, 
			filebox->column_titles,
			BC_FILEBOX_TOTAL_COLUMNS,
			0,
			0)
{ this->filebox = filebox; }

BC_FileBoxListBox::~BC_FileBoxListBox() {}

int BC_FileBoxListBox::handle_event()
{
	if(get_keypress() == ESC)
	{
		filebox->cancel_event();
		set_done(1);
	}
	else
	{
		char string[1024];
		int result = 0;
		
		get_selection(string, 0);

		if(filebox->want_directory)
		{
			if(filebox->submit_file(string)) result = 0;
		}
		else
		{
			if(!filebox->submit_file(string)) result = 1;
		}
		
		if(result)
		{
			filebox->ok_event();
			set_done(0);
		}
	}
	return 1;
return 0;
}

int BC_FileBoxListBox::selection_changed()
{
	char string[1024];
	get_selection(string, 0);
	filebox->textbox->update(string);
return 0;
}
