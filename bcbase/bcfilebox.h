#ifndef BCFILEBOX_H
#define BCFILEBOX_H


class BC_FileBoxOK;
class BC_FileBoxCancel;
class BC_FileBoxSubWindow;
class BC_FileBoxUseThis;
class BC_FileBoxListBox;
class BC_FileBoxTextBox;

#include "arraylist.h"
#include "bcbutton.h"
#include "bclistbox.h"
#include "bctextbox.h"
#include "bctitle.h"
#include "filesystem.inc"

#define BC_FILEBOX_TOTAL_COLUMNS 2

class BC_FileBox : public BC_Window
{
public:
	BC_FileBox(char *display,         // Set to "" to get default.
				char *init_directory, 
				char *title, 
				char *caption, 
				int show_all_files = 0, // Set to 1 to get hidden files. 
				int want_directory = 0);
	virtual ~BC_FileBox();

	friend BC_FileBoxListBox;
	friend BC_FileBoxOK;
	friend BC_FileBoxCancel;
	friend BC_FileBoxUseThis;

	int create_objects();

// ================================== user defined event handlers
	virtual int ok_event();
	virtual int cancel_event();
	int set_show_all();                // show all files
	char* get_directory();
	char* get_filename();

private:
// ===================================== data
// Data for the listbox
	int create_tables();             // Create information for listbox.
	int delete_tables();             // Delete all information for listbox.
	ArrayList<BC_ListBoxItem*> list_column[BC_FILEBOX_TOTAL_COLUMNS];  // Columns of array lists.
	const char *column_titles[BC_FILEBOX_TOTAL_COLUMNS];
	int column_width[BC_FILEBOX_TOTAL_COLUMNS];


	int submit_file(char *text);       // return 1 if directory 0 if file

	char directory[1024], filename[1024];
	int want_directory;
	char caption[1024];
	FileSystem *dir;
	BC_FileBox *filebox;
	BC_FileBoxOK *ok;
	BC_FileBoxCancel *cancel;
	BC_FileBoxUseThis *usethis;
	BC_FileBoxTextBox *textbox;
	BC_FileBoxListBox *listbox;
	BC_Title *directory_title;
	BC_FileBoxSubWindow *subwindow;
};

class BC_FileBoxOK : public BC_BigButton
{
public:
	BC_FileBoxOK(BC_FileBox *filebox);
	
	int handle_event();
	int keypress_event();
	
	BC_FileBox *filebox;
};

class BC_FileBoxCancel : public BC_BigButton
{
public:
	BC_FileBoxCancel(BC_FileBox *filebox);
	
	int handle_event();
	int keypress_event();
	
	BC_FileBox *filebox;
};

class BC_FileBoxUseThis : public BC_BigButton
{
public:
	BC_FileBoxUseThis(BC_FileBox *filebox);
	
	int handle_event();
	
	BC_FileBox *filebox;
};

class BC_FileBoxTextBox : public BC_TextBox
{
public:
	BC_FileBoxTextBox(BC_FileBox *filebox, char *text);
	
	int handle_event();
	
	BC_FileBox *filebox;
};

class BC_FileBoxDirectory : public BC_Title
{
public:
	BC_FileBoxDirectory(BC_FileBox *filebox, char *text);
};

class BC_FileBoxListBox : public BC_ListBox
{
public:
	BC_FileBoxListBox(BC_FileBox *filebox);
	virtual ~BC_FileBoxListBox();
	
	int handle_event();
	int selection_changed();
	
	BC_FileBox *filebox;
};

#endif

