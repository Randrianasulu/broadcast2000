#ifndef CHANNELEDIT_H
#define CHANNELEDIT_H

#include "bcbase.h"
#include "channel.inc"
#include "channelpicker.inc"
#include "mutex.h"

class ChannelEditWindow;

class ChannelEditThread : public Thread
{
public:
	ChannelEditThread(ChannelPicker *channel_picker);
	~ChannelEditThread();
	void run();
	int close_threads();

	Mutex completion;
	int in_progress;
	ChannelPicker *channel_picker;
	ArrayList <Channel*> *new_channels;
	ChannelEditWindow *window;
};

class ChannelEditList;
class ChannelEditEditThread;
class ChannelEditPictureThread;

class  ChannelEditWindow : public BC_Window
{
public:
	ChannelEditWindow(ChannelEditThread *thread, ChannelPicker *channel_picker);
	~ChannelEditWindow();

	int initialize();
	int close_event();
	int add_channel();  // Start the thread for adding a channel
	int delete_channel(int channel);
	int delete_channel(Channel *channel);
	int edit_channel();
	int edit_picture();
	int update_list();  // Synchronize the list box with the channel arrays
	int update_list(Channel *channel);  // Synchronize the list box and the channel
	int update_output();
	int move_channel_up();
	int move_channel_down();
	int change_channel_from_list(int channel_number);


	ArrayList<BC_ListBoxItem*> channel_list;
	ChannelEditList *list_box;
	ChannelEditThread *thread;
	ChannelPicker *channel_picker;
	ChannelEditEditThread *edit_thread;
	ChannelEditPictureThread *picture_thread;
};

class ChannelEditSelect : public BC_BigButton
{
public:
	ChannelEditSelect(int x, int y, ChannelEditWindow *window);
	~ChannelEditSelect();
	int handle_event();
	ChannelEditWindow *window;
};


class ChannelEditAdd : public BC_BigButton
{
public:
	ChannelEditAdd(int x, int y, ChannelEditWindow *window);
	~ChannelEditAdd();
	int handle_event();
	ChannelEditWindow *window;
};

class ChannelEditList : public BC_ListBox
{
public:
	ChannelEditList(int x, int y, ChannelEditWindow *window);
	~ChannelEditList();
	int handle_event();
	ChannelEditWindow *window;
	static char *column_titles[2];
};

class ChannelEditMoveUp : public BC_BigButton
{
public:
	ChannelEditMoveUp(int x, int y, ChannelEditWindow *window);
	~ChannelEditMoveUp();
	int handle_event();
	ChannelEditWindow *window;
};

class ChannelEditMoveDown : public BC_BigButton
{
public:
	ChannelEditMoveDown(int x, int y, ChannelEditWindow *window);
	~ChannelEditMoveDown();
	int handle_event();
	ChannelEditWindow *window;
};

class ChannelEditDel : public BC_BigButton
{
public:
	ChannelEditDel(int x, int y, ChannelEditWindow *window);
	~ChannelEditDel();
	int handle_event();
	ChannelEditWindow *window;
};

class ChannelEdit : public BC_BigButton
{
public:
	ChannelEdit(int x, int y, ChannelEditWindow *window);
	~ChannelEdit();
	int handle_event();
	ChannelEditWindow *window;
};

class ChannelEditPicture : public BC_BigButton
{
public:
	ChannelEditPicture(int x, int y, ChannelEditWindow *window);
	~ChannelEditPicture();
	int handle_event();
	ChannelEditWindow *window;
};


// ============================= Edit a single channel

class ChannelEditEditSource;
class ChannelEditEditWindow;

class ChannelEditEditThread : public Thread
{
public:
	ChannelEditEditThread(ChannelEditWindow *window, ChannelPicker *channel_picker);
	~ChannelEditEditThread();

	void run();
	int edit_channel(Channel *channel, int editing);
	int set_device();       // Set the device to the new channel
	int change_source(char *source_name);   // Change to the source matching the name
	int source_up();
	int source_down();
	int set_input(int value);
	int set_norm(int value);
	int set_freqtable(int value);
	char* value_to_freqtable(int value);
	char* value_to_norm(int value);
	char* value_to_input(int value);
	int close_threads();

	Channel new_channel;
	Channel *output_channel;
	ChannelPicker *channel_picker;
	ChannelEditWindow *window;
	ChannelEditEditSource *source_text;
	ChannelEditEditWindow *edit_window;
	int editing;   // Tells whether or not to delete the channel on cancel
	int in_progress;   // Allow only 1 thread at a time
	Mutex completion;
};

class ChannelEditEditWindow : public BC_Window
{
public:
	ChannelEditEditWindow(ChannelEditEditThread *thread, ChannelEditWindow *window);
	~ChannelEditEditWindow();
	int initialize(Channel *channel);

	ChannelEditEditThread *thread;
	ChannelEditWindow *window;
	Channel *new_channel;
};

class ChannelEditEditTitle : public BC_TextBox
{
public:
	ChannelEditEditTitle(int x, int y, ChannelEditEditThread *thread);
	~ChannelEditEditTitle();
	int handle_event();
	ChannelEditEditThread *thread;
};

class ChannelEditEditSource : public BC_TextBox
{
public:
	ChannelEditEditSource(int x, int y, ChannelEditEditThread *thread);
	~ChannelEditEditSource();
	int handle_event();
	ChannelEditEditThread *thread;
};

class ChannelEditEditSourceUp : public BC_UpTriangleButton
{
public:
	ChannelEditEditSourceUp(int x, int y, ChannelEditEditThread *thread);
	~ChannelEditEditSourceUp();
	int handle_event();
	ChannelEditEditThread *thread;
};

class ChannelEditEditSourceDown : public BC_DownTriangleButton
{
public:
	ChannelEditEditSourceDown(int x, int y, ChannelEditEditThread *thread);
	~ChannelEditEditSourceDown();
	int handle_event();
	ChannelEditEditThread *thread;
};

class ChannelEditEditInput : public BC_PopupMenu
{
public:
	ChannelEditEditInput(int x, int y, ChannelEditEditThread *thread);
	~ChannelEditEditInput();
	int add_items();
	int handle_event();
	ChannelEditEditThread *thread;
};

class ChannelEditEditInputItem : public BC_PopupItem
{
public:
	ChannelEditEditInputItem(ChannelEditEditThread *thread, char *text, int value);
	~ChannelEditEditInputItem();
	int handle_event();
	ChannelEditEditThread *thread;
	int value;
};

class ChannelEditEditNorm : public BC_PopupMenu
{
public:
	ChannelEditEditNorm(int x, int y, ChannelEditEditThread *thread);
	~ChannelEditEditNorm();
	int add_items();
	ChannelEditEditThread *thread;
};

class ChannelEditEditNormItem : public BC_PopupItem
{
public:
	ChannelEditEditNormItem(ChannelEditEditThread *thread, char *text, int value);
	~ChannelEditEditNormItem();
	int handle_event();
	ChannelEditEditThread *thread;
	int value;
};

class ChannelEditEditFreqtable : public BC_PopupMenu
{
public:
	ChannelEditEditFreqtable(int x, int y, ChannelEditEditThread *thread);
	~ChannelEditEditFreqtable();

	int add_items();

	ChannelEditEditThread *thread;
};

class ChannelEditEditFreqItem : public BC_PopupItem
{
public:
	ChannelEditEditFreqItem(ChannelEditEditThread *thread, char *text, int value);
	~ChannelEditEditFreqItem();

	int handle_event();
	ChannelEditEditThread *thread;
	int value;
};

class ChannelEditEditFine : public BC_ISlider
{
public:
	ChannelEditEditFine(int x, int y, ChannelEditEditThread *thread);
	~ChannelEditEditFine();
	int handle_event();
	ChannelEditEditThread *thread;
};

// =================== Edit the picture quality


class ChannelEditPictureWindow;

class ChannelEditPictureThread : public Thread
{
public:
	ChannelEditPictureThread(ChannelPicker *channel_picker, ChannelEditWindow *window);
	~ChannelEditPictureThread();

	void run();
	int close_threads();
	int edit_picture();

	int in_progress;   // Allow only 1 thread at a time
	Mutex completion;
	ChannelPicker *channel_picker;
	ChannelEditWindow *window;
	ChannelEditPictureWindow *edit_window;
};

class ChannelEditPictureWindow : public BC_Window
{
public:
	ChannelEditPictureWindow(ChannelEditPictureThread *thread, ChannelPicker *channel_picker);
	~ChannelEditPictureWindow();
	int initialize();

	ChannelEditPictureThread *thread;
	ChannelPicker *channel_picker;
};

class ChannelEditBright : public BC_IPot
{
public:
	ChannelEditBright(int x, int y, ChannelPicker *channel_picker, int value);
	~ChannelEditBright();
	int handle_event();
	ChannelPicker *channel_picker;
};

class ChannelEditContrast : public BC_IPot
{
public:
	ChannelEditContrast(int x, int y, ChannelPicker *channel_picker, int value);
	~ChannelEditContrast();
	int handle_event();
	ChannelPicker *channel_picker;
};

class ChannelEditColor : public BC_IPot
{
public:
	ChannelEditColor(int x, int y, ChannelPicker *channel_picker, int value);
	~ChannelEditColor();
	int handle_event();
	ChannelPicker *channel_picker;
};

class ChannelEditHue : public BC_IPot
{
public:
	ChannelEditHue(int x, int y, ChannelPicker *channel_picker, int value);
	~ChannelEditHue();
	int handle_event();
	ChannelPicker *channel_picker;
};

class ChannelEditWhiteness : public BC_IPot
{
public:
	ChannelEditWhiteness(int x, int y, ChannelPicker *channel_picker, int value);
	~ChannelEditWhiteness();
	int handle_event();
	ChannelPicker *channel_picker;
};



#endif
