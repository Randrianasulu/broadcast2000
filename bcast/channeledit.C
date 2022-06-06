#include <string.h>
#include "channel.h"
#include "channeledit.h"
#include "channelpicker.h"
#include "okbutton.h"
#include "chantables.h"
#include "record.h"
#include "recordengine.h"


ChannelEditThread::ChannelEditThread(ChannelPicker *channel_picker)
 : Thread()
{
	this->channel_picker = channel_picker;
	in_progress = 0;
	window = 0;
}
ChannelEditThread::~ChannelEditThread()
{
}

void ChannelEditThread::run()
{
	int i;

	if(in_progress) return;
	in_progress = 1;
	completion.lock();

// Copy master channel list to temporary.
	new_channels = new ArrayList <Channel*>;
	for(i = 0; i < channel_picker->channeldb->total; i++)
	{
		new_channels->append(new Channel(channel_picker->channeldb->values[i]));
	}

// Run the window using the temporary.
	ChannelEditWindow window(this, channel_picker);
	window.initialize();
	this->window = &window;
	int result = window.run_window();
	this->window = 0;

	if(!result)
	{
// Copy new channels to master list
		for(i = 0; i < new_channels->total && i < channel_picker->channeldb->total; i++)
		{
			*(channel_picker->channeldb->values[i]) = *(new_channels->values[i]);
		}

		for( ; i < new_channels->total ; i++)
		{
			channel_picker->channeldb->append(new Channel(new_channels->values[i]));
		}

		for( ; i < channel_picker->channeldb->total; i++)
		{
			delete channel_picker->channeldb->values[i];
			channel_picker->channeldb->remove_number(i);
		}
	}
	else
	{
// Rejected.
	}
	window.edit_thread->close_threads();
	window.picture_thread->close_threads();

	for(i = 0; i < new_channels->total; i++)
	{
		delete new_channels->values[i];
	}
	new_channels->remove_all();
	delete new_channels;
	completion.unlock();
	in_progress = 0;
}

int ChannelEditThread::close_threads()
{
	if(in_progress && window)
	{
		window->edit_thread->close_threads();
		window->picture_thread->close_threads();
		window->set_done(1);
		completion.lock();
		completion.unlock();
	}
return 0;
}


ChannelEditWindow::ChannelEditWindow(ChannelEditThread *thread, ChannelPicker *channel_picker)
 : BC_Window(ICONNAME ": Channels", 330, 330, 330, 330, 0, 0)
{
	this->thread = thread;
	this->channel_picker = channel_picker;
}
ChannelEditWindow::~ChannelEditWindow()
{
	int i;
	for(i = 0; i < channel_list.total; i++)
	{
		delete channel_list.values[i];
	}
	channel_list.remove_all();
	delete edit_thread;
	delete picture_thread;
}

int ChannelEditWindow::initialize()
{
	int x = 10, y = 10, i;
	char string[1024];

// Create channel list
	for(i = 0; i < thread->new_channels->total; i++)
	{
		channel_list.append(new BC_ListBoxItem(thread->new_channels->values[i]->title));
	}

	add_tool(list_box = new ChannelEditList(x, y, this));
	x += 200;
	add_tool(new ChannelEditSelect(x, y, this));
	y += 30;
	add_tool(new ChannelEditAdd(x, y, this));
	y += 30;
	add_tool(new ChannelEdit(x, y, this));
	y += 30;
	add_tool(new ChannelEditMoveUp(x, y, this));
	y += 30;
	add_tool(new ChannelEditMoveDown(x, y, this));
	y += 30;
	add_tool(new ChannelEditDel(x, y, this));
	y += 30;
	add_tool(new ChannelEditPicture(x, y, this));
	y += 100;
	x = 10;
	add_tool(new OKButton(x, y));
	x += 150;
	add_tool(new CancelButton(x, y));
	
	edit_thread = new ChannelEditEditThread(this, channel_picker);
	picture_thread = new ChannelEditPictureThread(channel_picker, this);
return 0;
}

int ChannelEditWindow::close_event()
{
	set_done(0);
return 0;
}

int ChannelEditWindow::add_channel()
{
	Channel *new_channel;
	
	new_channel = new Channel;
	if(thread->new_channels->total) *new_channel = *(thread->new_channels->values[thread->new_channels->total - 1]);
	channel_list.append(new BC_ListBoxItem(new_channel->title));
	thread->new_channels->append(new_channel);
	update_list();
	edit_thread->edit_channel(new_channel, 0);
return 0;
}

int ChannelEditWindow::update_list()
{
	list_box->set_contents(&channel_list, 0, 1);
return 0;
}

int ChannelEditWindow::update_list(Channel *channel)
{
	int i;
	for(i = 0; i < thread->new_channels->total; i++)
		if(thread->new_channels->values[i] == channel) break;

	if(i < thread->new_channels->total)
	{
		channel_list.values[i]->set_text(channel->title);
	}

	update_list();
return 0;
}


int ChannelEditWindow::edit_channel()
{
	if(list_box->get_selection_number() > -1)
		edit_thread->edit_channel(thread->new_channels->values[list_box->get_selection_number()], 1);
return 0;
}

int ChannelEditWindow::edit_picture()
{
	picture_thread->edit_picture();
return 0;
}

int ChannelEditWindow::delete_channel(int number)
{
	delete thread->new_channels->values[number];
	channel_list.remove_number(number);
	thread->new_channels->remove_number(number);
	update_list();
return 0;
}

int ChannelEditWindow::delete_channel(Channel *channel)
{
	int i;
	for(i = 0; i < thread->new_channels->total; i++)
	{
		if(thread->new_channels->values[i] == channel)
		{
			break;
		}
	}
	if(i < thread->new_channels->total) delete_channel(i);
return 0;
}

int ChannelEditWindow::move_channel_up()
{
	if(list_box->get_selection_number() > -1)
	{
		int number2 = list_box->get_selection_number();
		int number1 = number2 - 1;
		Channel *temp;
		BC_ListBoxItem *temp_text;

		if(number1 < 0) number1 = thread->new_channels->total - 1;

		temp = thread->new_channels->values[number1];
		thread->new_channels->values[number1] = thread->new_channels->values[number2];
		thread->new_channels->values[number2] = temp;
		temp_text = channel_list.values[number1];
		channel_list.values[number1] = channel_list.values[number2];
		channel_list.values[number2] = temp_text;
		list_box->set_contents(&channel_list, 0, 1, list_box->get_yposition(), number1);
	}
return 0;
}

int ChannelEditWindow::move_channel_down()
{
	if(list_box->get_selection_number() > -1)
	{
		int number2 = list_box->get_selection_number();
		int number1 = number2 + 1;
		Channel *temp;
		BC_ListBoxItem *temp_text;

		if(number1 > thread->new_channels->total - 1) number1 = 0;

		temp = thread->new_channels->values[number1];
		thread->new_channels->values[number1] = thread->new_channels->values[number2];
		thread->new_channels->values[number2] = temp;
		temp_text = channel_list.values[number1];
		channel_list.values[number1] = channel_list.values[number2];
		channel_list.values[number2] = temp_text;
		list_box->set_contents(&channel_list, 0, 1, list_box->get_yposition(), number1);
	}
return 0;
}

int ChannelEditWindow::change_channel_from_list(int channel_number)
{
	Channel *channel;
	if(channel_number > -1 && channel_number < thread->new_channels->total)
	{
		channel_picker->window->lock_window();
		channel_picker->engine->record->current_channel = channel_number;
		channel = thread->new_channels->values[channel_number];
		channel_picker->engine->change_channel(channel);
		channel_picker->channel_button->update(channel->title);
		channel_picker->window->unlock_window();
	}
return 0;
}

ChannelEditSelect::ChannelEditSelect(int x, int y, ChannelEditWindow *window)
 : BC_BigButton(x, y, "Select")
{
	this->window = window;
}
ChannelEditSelect::~ChannelEditSelect()
{
}
int ChannelEditSelect::handle_event()
{
	window->change_channel_from_list(window->list_box->get_selection_number());
return 0;
}

ChannelEditAdd::ChannelEditAdd(int x, int y, ChannelEditWindow *window)
 : BC_BigButton(x, y, "Add...")
{
	this->window = window;
}
ChannelEditAdd::~ChannelEditAdd()
{
}
int ChannelEditAdd::handle_event()
{
	window->add_channel();
return 0;
}

ChannelEditList::ChannelEditList(int x, int y, ChannelEditWindow *window)
 : BC_ListBox(x, y, 170, 250, 
			&(window->channel_list),
			0, // Titles for columns.  Set to 0 for no titles
			1, // Total columns.
			0, // Pixel of top of window.
			-1)
{
	this->window = window;
	stay_highlighted();
}
ChannelEditList::~ChannelEditList()
{
}
int ChannelEditList::handle_event()
{
	window->change_channel_from_list(window->list_box->get_selection_number());
return 0;
}

ChannelEditMoveUp::ChannelEditMoveUp(int x, int y, ChannelEditWindow *window)
 : BC_BigButton(x, y, "Move Up")
{
	this->window = window;
}
ChannelEditMoveUp::~ChannelEditMoveUp()
{
}
int ChannelEditMoveUp::handle_event()
{
	lock_window();
	window->move_channel_up();
	unlock_window();
return 0;
}

ChannelEditMoveDown::ChannelEditMoveDown(int x, int y, ChannelEditWindow *window)
 : BC_BigButton(x, y, "Move Down")
{
	this->window = window;
}
ChannelEditMoveDown::~ChannelEditMoveDown()
{
}
int ChannelEditMoveDown::handle_event()
{
	lock_window();
	window->move_channel_down();
	unlock_window();
return 0;
}

ChannelEditDel::ChannelEditDel(int x, int y, ChannelEditWindow *window)
 : BC_BigButton(x, y, "Delete")
{
	this->window = window;
}
ChannelEditDel::~ChannelEditDel()
{
}
int ChannelEditDel::handle_event()
{
	if(window->list_box->get_selection_number() > -1) window->delete_channel(window->list_box->get_selection_number());
return 0;
}

ChannelEdit::ChannelEdit(int x, int y, ChannelEditWindow *window)
 : BC_BigButton(x, y, "Edit...")
{
	this->window = window;
}
ChannelEdit::~ChannelEdit()
{
}
int ChannelEdit::handle_event()
{
	window->edit_channel();
return 0;
}

ChannelEditPicture::ChannelEditPicture(int x, int y, ChannelEditWindow *window)
 : BC_BigButton(x, y, "Picture...")
{
	this->window = window;
}
ChannelEditPicture::~ChannelEditPicture()
{
}
int ChannelEditPicture::handle_event()
{
	window->edit_picture();
return 0;
}



// ================================= Edit a single channel



ChannelEditEditThread::ChannelEditEditThread(ChannelEditWindow *window, ChannelPicker *channel_picker)
 : Thread()
{
	this->window = window;
	this->channel_picker = channel_picker;
	in_progress = 0;
	edit_window = 0;
	editing = 0;
}

ChannelEditEditThread::~ChannelEditEditThread()
{
}

int ChannelEditEditThread::close_threads()
{
	if(edit_window)
	{
		edit_window->set_done(1);
		completion.lock();
		completion.unlock();
	}
return 0;
}

int ChannelEditEditThread::edit_channel(Channel *channel, int editing)
{
	if(in_progress) return 1;
	in_progress = 1;

	completion.lock();
	this->editing = editing;
	this->output_channel = channel;
	this->new_channel = *output_channel;
	Thread::synchronous = 0;
	Thread::start();
return 0;
}

const char *ChannelEditEditThread::value_to_freqtable(int value)
{
	switch(value)
	{
		case NTSC_BCAST:
			return "NTSC_BCAST";
			break;
		case NTSC_CABLE:
			return "NTSC_CABLE";
			break;
		case NTSC_HRC:
			return "NTSC_HRC";
			break;
		case NTSC_BCAST_JP:
			return "NTSC_BCAST_JP";
			break;
		case NTSC_CABLE_JP:
			return "NTSC_CABLE_JP";
			break;
		case PAL_AUSTRALIA:
			return "PAL_AUSTRALIA";
			break;
		case PAL_EUROPE:
			return "PAL_EUROPE";
			break;
		case PAL_E_EUROPE:
			return "PAL_E_EUROPE";
			break;
		case PAL_ITALY:
			return "PAL_ITALY";
			break;
		case PAL_IRELAND:
			return "PAL_IRELAND";
			break;
		case PAL_NEWZEALAND:
			return "PAL_NEWZEALAND";
			break;
	}
return "NTSC_CABLE";
}

const char* ChannelEditEditThread::value_to_norm(int value)
{
	switch(value)
	{
		case NTSC:
			return "NTSC";
			break;
		case PAL:
			return "PAL";
			break;
		case SECAM:
			return "SECAM";
			break;
	}
return "NTSC"; //default
}

const char* ChannelEditEditThread::value_to_input(int value)
{
	return channel_picker->engine->get_video_inputs()->values[value];
}

int ChannelEditEditThread::set_device()
{
	return channel_picker->engine->change_channel(&new_channel);
return 0;
}

int ChannelEditEditThread::change_source(const char *source_name)
{
	int i, result;
	for(i = 0; i < chanlists[new_channel.freqtable].count; i++)
	{
		if(!strcasecmp(chanlists[new_channel.freqtable].list[i].name, source_name))
		{
			new_channel.entry = i;
			i = chanlists[new_channel.freqtable].count;
			set_device();
		}
	}
return 0;
}

int ChannelEditEditThread::source_up()
{
	new_channel.entry++;
	if(new_channel.entry > chanlists[new_channel.freqtable].count - 1) new_channel.entry = 0;
	source_text->update(chanlists[new_channel.freqtable].list[new_channel.entry].name);
	set_device();
return 0;
}

int ChannelEditEditThread::source_down()
{
	new_channel.entry--;
	if(new_channel.entry < 0) new_channel.entry = chanlists[new_channel.freqtable].count - 1;
	source_text->update(chanlists[new_channel.freqtable].list[new_channel.entry].name);
	set_device();
return 0;
}

int ChannelEditEditThread::set_input(int value)
{
	new_channel.input = value;
	set_device();
return 0;
}

int ChannelEditEditThread::set_norm(int value)
{
	new_channel.norm = value;
	set_device();
return 0;
}

int ChannelEditEditThread::set_freqtable(int value)
{
	new_channel.freqtable = value;
	if(new_channel.entry > chanlists[new_channel.freqtable].count - 1) new_channel.entry = 0;
	source_text->update(chanlists[new_channel.freqtable].list[new_channel.entry].name);
	set_device();
return 0;
}

void ChannelEditEditThread::run()
{
	ChannelEditEditWindow edit_window(this, window);
	edit_window.initialize(&new_channel);
	this->edit_window = &edit_window;
	int result = edit_window.run_window();
	this->edit_window = 0;
	if(!result)
	{
		*output_channel = new_channel;
		window->lock_window();
		window->update_list(output_channel);
		window->unlock_window();
	}
	else
	{
		if(!editing)
		{
			window->lock_window();
			window->delete_channel(output_channel);
			window->unlock_window();
		}
	}
	editing = 0;
	completion.unlock();
	in_progress = 0;
}

ChannelEditEditWindow::ChannelEditEditWindow(ChannelEditEditThread *thread, ChannelEditWindow *window)
 : BC_Window(ICONNAME ": Edit Channel", 390, 235, 390, 235)
{
	this->window = window;
	this->thread = thread;
}
ChannelEditEditWindow::~ChannelEditEditWindow()
{
}
int ChannelEditEditWindow::initialize(Channel *channel)
{
	this->new_channel = channel;

	int x = 10, y = 10;
	add_tool(new BC_Title(x, y, "Title:"));
	add_tool(new ChannelEditEditTitle(x, y + 20, thread));
	x += 170;
	add_tool(new BC_Title(x, y, "Source:"));
	y += 20;
	add_tool(thread->source_text = new ChannelEditEditSource(x, y, thread));
	x += 160;
	add_tool(new ChannelEditEditSourceUp(x, y, thread));
	x += 20;
	add_tool(new ChannelEditEditSourceDown(x, y, thread));
	y += 40;
	x = 10;
	add_tool(new BC_Title(x, y, "Fine:"));
	add_tool(new ChannelEditEditFine(x + 130, y, thread));
	y += 30;
	add_tool(new BC_Title(x, y, "Norm:"));
	add_tool(new ChannelEditEditNorm(x + 130, y, thread));
	y += 30;
	add_tool(new BC_Title(x, y, "Frequency table:"));
	add_tool(new ChannelEditEditFreqtable(x + 130, y, thread));
	y += 30;
	add_tool(new BC_Title(x, y, "Input:"));
	add_tool(new ChannelEditEditInput(x + 130, y, thread));
	y += 30;
	add_tool(new OKButton(x, y));
	x += 200;
	add_tool(new CancelButton(x, y));
return 0;
}

ChannelEditEditTitle::ChannelEditEditTitle(int x, int y, ChannelEditEditThread *thread)
 : BC_TextBox(x, y, 150, thread->new_channel.title)
{
	this->thread = thread;
}
ChannelEditEditTitle::~ChannelEditEditTitle()
{
}
int ChannelEditEditTitle::handle_event()
{
	if(strlen(get_text()) < 1024)
		strcpy(thread->new_channel.title, get_text());
return 0;
}


ChannelEditEditSource::ChannelEditEditSource(int x, int y, ChannelEditEditThread *thread)
 : BC_TextBox(x, y, 150, chanlists[thread->new_channel.freqtable].list[thread->new_channel.entry].name)
{
	this->thread = thread;
}

ChannelEditEditSource::~ChannelEditEditSource()
{
}
int ChannelEditEditSource::handle_event()
{
	thread->change_source(get_text());
return 0;
}


ChannelEditEditSourceUp::ChannelEditEditSourceUp(int x, int y, ChannelEditEditThread *thread)
 : BC_UpTriangleButton(x, y, 20, 20)
{
	this->thread = thread;
}
ChannelEditEditSourceUp::~ChannelEditEditSourceUp()
{
}
int ChannelEditEditSourceUp::handle_event()
{
	thread->source_up();
return 0;
}

ChannelEditEditSourceDown::ChannelEditEditSourceDown(int x, int y, ChannelEditEditThread *thread)
 : BC_DownTriangleButton(x, y, 20, 20)
{
	this->thread = thread;
}
ChannelEditEditSourceDown::~ChannelEditEditSourceDown()
{
}
int ChannelEditEditSourceDown::handle_event()
{
	thread->source_down();
return 0;
}

ChannelEditEditInput::ChannelEditEditInput(int x, int y, ChannelEditEditThread *thread)
 : BC_PopupMenu(x, y, 150, thread->value_to_input(thread->new_channel.input))
{
	this->thread = thread;
}
ChannelEditEditInput::~ChannelEditEditInput()
{
}
int ChannelEditEditInput::add_items()
{
	ArrayList<char*> *inputs;
	inputs = thread->channel_picker->engine->get_video_inputs();
	
	if(inputs)
		for(int i = 0; i < inputs->total; i++)
		{
			add_item(new ChannelEditEditInputItem(thread, inputs->values[i], i));
		}
return 0;
}
int ChannelEditEditInput::handle_event()
{
return 0;
}

ChannelEditEditInputItem::ChannelEditEditInputItem(ChannelEditEditThread *thread, char *text, int value)
 : BC_PopupItem(text)
{
	this->thread = thread;
	this->value = value;
}
ChannelEditEditInputItem::~ChannelEditEditInputItem()
{
}
int ChannelEditEditInputItem::handle_event()
{
	get_menu()->update(get_text());
	thread->set_input(value);
return 0;
}

ChannelEditEditNorm::ChannelEditEditNorm(int x, int y, ChannelEditEditThread *thread)
 : BC_PopupMenu(x, y, 100, thread->value_to_norm(thread->new_channel.norm))
{
	this->thread = thread;
}
ChannelEditEditNorm::~ChannelEditEditNorm()
{
}
int ChannelEditEditNorm::add_items()
{
	add_item(new ChannelEditEditNormItem(thread, thread->value_to_norm(NTSC), NTSC));
	add_item(new ChannelEditEditNormItem(thread, thread->value_to_norm(PAL), PAL));
	add_item(new ChannelEditEditNormItem(thread, thread->value_to_norm(SECAM), SECAM));
return 0;
}


ChannelEditEditNormItem::ChannelEditEditNormItem(ChannelEditEditThread *thread, char *text, int value)
 : BC_PopupItem(text)
{
	this->value = value;
	this->thread = thread;
}
ChannelEditEditNormItem::~ChannelEditEditNormItem()
{
}
int ChannelEditEditNormItem::handle_event()
{
	get_menu()->update(get_text());
	thread->set_norm(value);
return 0;
}


ChannelEditEditFreqtable::ChannelEditEditFreqtable(int x, int y, ChannelEditEditThread *thread)
 : BC_PopupMenu(x, y, 150, thread->value_to_freqtable(thread->new_channel.freqtable))
{
	this->thread = thread;
}
ChannelEditEditFreqtable::~ChannelEditEditFreqtable()
{
}
int ChannelEditEditFreqtable::add_items()
{
	add_item(new ChannelEditEditFreqItem(thread, thread->value_to_freqtable(NTSC_BCAST), NTSC_BCAST));
	add_item(new ChannelEditEditFreqItem(thread, thread->value_to_freqtable(NTSC_CABLE), NTSC_CABLE));
	add_item(new ChannelEditEditFreqItem(thread, thread->value_to_freqtable(NTSC_HRC), NTSC_HRC));
	add_item(new ChannelEditEditFreqItem(thread, thread->value_to_freqtable(NTSC_BCAST_JP), NTSC_BCAST_JP));
	add_item(new ChannelEditEditFreqItem(thread, thread->value_to_freqtable(NTSC_CABLE_JP), NTSC_CABLE_JP));
	add_item(new ChannelEditEditFreqItem(thread, thread->value_to_freqtable(PAL_AUSTRALIA), PAL_AUSTRALIA));
	add_item(new ChannelEditEditFreqItem(thread, thread->value_to_freqtable(PAL_EUROPE), PAL_EUROPE));
	add_item(new ChannelEditEditFreqItem(thread, thread->value_to_freqtable(PAL_E_EUROPE), PAL_E_EUROPE));
	add_item(new ChannelEditEditFreqItem(thread, thread->value_to_freqtable(PAL_ITALY), PAL_ITALY));
	add_item(new ChannelEditEditFreqItem(thread, thread->value_to_freqtable(PAL_IRELAND), PAL_IRELAND));
	add_item(new ChannelEditEditFreqItem(thread, thread->value_to_freqtable(PAL_NEWZEALAND), PAL_NEWZEALAND));
return 0;
}

ChannelEditEditFreqItem::ChannelEditEditFreqItem(ChannelEditEditThread *thread, char *text, int value)
 : BC_PopupItem(text)
{
	this->value = value;
	this->thread = thread;
}
ChannelEditEditFreqItem::~ChannelEditEditFreqItem()
{
}
int ChannelEditEditFreqItem::handle_event()
{
	get_menu()->update(get_text());
	thread->set_freqtable(value);
return 0;
}



ChannelEditEditFine::ChannelEditEditFine(int x, int y, ChannelEditEditThread *thread)
 : BC_ISlider(x, y, 240, 20, 240, thread->new_channel.fine_tune, -100, 100, 0, 1)
{
	this->thread = thread;
}
ChannelEditEditFine::~ChannelEditEditFine()
{
}
int ChannelEditEditFine::handle_event()
{
	thread->new_channel.fine_tune = get_value();
	thread->set_device();
return 0;
}


// ========================== picture quality

ChannelEditPictureThread::ChannelEditPictureThread(ChannelPicker *channel_picker, ChannelEditWindow *window)
 : Thread()
{
	this->channel_picker = channel_picker;
	this->window = window;
	in_progress = 0;
	edit_window = 0;
}
ChannelEditPictureThread::~ChannelEditPictureThread()
{
}

int ChannelEditPictureThread::edit_picture()
{
	if(in_progress) return 1;
	in_progress = 1;
	completion.lock();
	Thread::synchronous = 0;
	Thread::start();
return 0;
}

void ChannelEditPictureThread::run()
{
	ChannelEditPictureWindow edit_window(this, channel_picker);
	edit_window.initialize();
	this->edit_window = &edit_window;
	int result = edit_window.run_window();
	this->edit_window = 0;
	completion.unlock();
	in_progress = 0;
}

int ChannelEditPictureThread::close_threads()
{
	if(edit_window)
	{
		edit_window->set_done(1);
		completion.lock();
		completion.unlock();
	}
return 0;
}


ChannelEditPictureWindow::ChannelEditPictureWindow(ChannelEditPictureThread *thread, ChannelPicker *channel_picker)
 : BC_Window(ICONNAME ": Picture", 200, 220, 200, 220)
{
	this->thread = thread;
	this->channel_picker = channel_picker;
}
ChannelEditPictureWindow::~ChannelEditPictureWindow()
{
}
int ChannelEditPictureWindow::initialize()
{
	int x = 10, y = 10;
	add_tool(new BC_Title(x, y + 10, "Brightness:"));
	add_tool(new ChannelEditBright(x + 100, y, channel_picker, channel_picker->engine->record->video_brightness));
	y += 30;
	add_tool(new BC_Title(x, y + 10, "Contrast:"));
	add_tool(new ChannelEditContrast(x + 135, y, channel_picker, channel_picker->engine->record->video_contrast));
	y += 30;
	add_tool(new BC_Title(x, y + 10, "Color:"));
	add_tool(new ChannelEditColor(x + 100, y, channel_picker, channel_picker->engine->record->video_color));
	y += 30;
	add_tool(new BC_Title(x, y + 10, "Hue:"));
	add_tool(new ChannelEditHue(x + 135, y, channel_picker, channel_picker->engine->record->video_hue));
	y += 30;
	add_tool(new BC_Title(x, y + 10, "Whiteness:"));
	add_tool(new ChannelEditWhiteness(x + 100, y, channel_picker, channel_picker->engine->record->video_whiteness));
	y += 50;
	x += 70;
	add_tool(new OKButton(x, y));
return 0;
}



ChannelEditBright::ChannelEditBright(int x, int y, ChannelPicker *channel_picker, int value)
 : BC_IPot(x, y, 35, 35, value, -100, 100, LTGREY, MEGREY)
{
	this->channel_picker = channel_picker;
}
ChannelEditBright::~ChannelEditBright() {}
int ChannelEditBright::handle_event()
{
	channel_picker->set_brightness(get_value());
return 0;
}

ChannelEditContrast::ChannelEditContrast(int x, int y, ChannelPicker *channel_picker, int value)
 : BC_IPot(x, y, 35, 35, value, -100, 100, LTGREY, MEGREY)
{
	this->channel_picker = channel_picker;
}
ChannelEditContrast::~ChannelEditContrast() {}
int ChannelEditContrast::handle_event()
{
	channel_picker->set_contrast(get_value());
return 0;
}

ChannelEditColor::ChannelEditColor(int x, int y, ChannelPicker *channel_picker, int value)
 : BC_IPot(x, y, 35, 35, value, -100, 100, LTGREY, MEGREY)
{
	this->channel_picker = channel_picker;
}
ChannelEditColor::~ChannelEditColor() {}
int ChannelEditColor::handle_event()
{
	channel_picker->set_color(get_value());
return 0;
}

ChannelEditHue::ChannelEditHue(int x, int y, ChannelPicker *channel_picker, int value)
 : BC_IPot(x, y, 35, 35, value, -100, 100, LTGREY, MEGREY)
{
	this->channel_picker = channel_picker;
}
ChannelEditHue::~ChannelEditHue() {}
int ChannelEditHue::handle_event()
{
	channel_picker->set_hue(get_value());
return 0;
}

ChannelEditWhiteness::ChannelEditWhiteness(int x, int y, ChannelPicker *channel_picker, int value)
 : BC_IPot(x, y, 35, 35, value, -100, 100, LTGREY, MEGREY)
{
	this->channel_picker = channel_picker;
}
ChannelEditWhiteness::~ChannelEditWhiteness() {}
int ChannelEditWhiteness::handle_event()
{
	channel_picker->set_whiteness(get_value());
return 0;
}
