#include "synthesizer.h"
#include "synthmenu.h"
#include "synthwindow.h"


SynthThread::SynthThread(Synth *synth)
 : Thread()
{
	this->synth = synth;
	synchronous = 1; // make thread wait for join
	gui_started.lock(); // make plugin wait for startup
}

SynthThread::~SynthThread()
{
}
	
void SynthThread::run()
{
	window = new SynthWindow(synth);
	window->create_objects();
	gui_started.unlock(); // make plugin wait for startup
	window->run_window();
// defaults are saves by synthesizer before this
	delete window;
}






SynthWindow::SynthWindow(Synth *synth)
 : BC_Window("", MEGREY, synth->gui_string, 380, synth->h, 380, 0, 0, !synth->show_initially)
{ this->synth = synth; }

SynthWindow::~SynthWindow()
{
}

int SynthWindow::create_objects()
{
	add_tool(menu = new SynthMenu(synth, this));
// menu loads its defaults here
	menu->create_objects(synth->defaults);
// load our defaults
	load_defaults(synth->defaults);

	int x = 10, y = 30, i;
	add_tool(new BC_Title(x, y, "Waveform"));
	x += 100;
	//add_tool(new BC_Title(x, y, "Zoom:"));
	x += 50;
	//add_tool(new SynthCanvasZoomout(this, x, y));
	x += 20;
	//add_tool(new SynthCanvasZoomin(this, x, y));
	x += 70;
	add_tool(new BC_Title(x, y, "Wave Function"));
	//y += 5;
	y += 20;
	x = 10;
	add_tool(canvas = new SynthCanvas(synth, this, x, y, 230, 151));
	
	x += 240;
	char string[1024];
	waveform_to_text(string, synth->wavefunction);

	add_tool(waveform = new SynthWaveForm(synth, x, y, string));
	y += 30;

	add_tool(new BC_Title(x, y, "Base Frequency"));
	y += 30;
	add_tool(base_freq = new SynthBaseFreq(synth, x, y));
	x += 80;
	add_tool(freqpot = new SynthFreqPot(synth, this, x, y - 10));
	base_freq->freq_pot = freqpot;
	freqpot->freq_text = base_freq;

	x -= 80;
	y += 40;
	add_tool(clearbutton = new SynthClear(synth, x, y));

	x = 50;  y = 210;
	add_tool(new BC_Title(x, y, "Level")); x += 75;
	add_tool(new BC_Title(x, y, "Phase")); x += 75;
	add_tool(new BC_Title(x, y, "Harmonic"));

	y += 20; x = 10;
	add_subwindow(subwindow = new SynthSubWindow(synth, x, y, 265, get_h() - y));
	x += 265;
	add_tool(scroll = new SynthScroll(synth, this, x, y, get_h() - y));
	
	x += 20;
	add_tool(new SynthAddOsc(synth, this, x, y));
	y += 30;
	add_tool(new SynthDelOsc(synth, this, x, y));
return 0;
}

int SynthWindow::load_defaults(Defaults *defaults)
{
	canvas_zoom = defaults->get("CANVASZOOM", (float)1);
return 0;
}

int SynthWindow::save_defaults(Defaults *defaults)
{
	defaults->update("CANVASZOOM", canvas_zoom);
	menu->save_defaults(defaults);
return 0;
}

int SynthWindow::update_gui()
{
	freqpot->update(synth->base_freq);
	base_freq->update((int)synth->base_freq);

	char string[1024];
	waveform_to_text(string, synth->wavefunction);
	waveform->update(string);
	
	canvas->update();
return 0;
}

int SynthWindow::waveform_to_text(char *text, int waveform)
{
	switch(waveform)
	{
		case SINE:            sprintf(text, "Sine");           break;
		case SAWTOOTH:        sprintf(text, "Sawtooth");       break;
		case SQUARE:          sprintf(text, "Square");         break;
		case TRIANGLE:        sprintf(text, "Triangle");       break;
		case PULSE:           sprintf(text, "Pulse");       break;
		case NOISE:           sprintf(text, "Noise");       break;
	}
return 0;
}


int SynthWindow::close_event()
{
	hide_window();
	synth->send_hide_gui();
return 0;
}

int SynthWindow::resize_event(int w, int h)
{
	subwindow->resize_window(subwindow->get_x(), subwindow->get_y(), subwindow->get_w(), h - subwindow->get_y());
	scroll->set_size(scroll->x, scroll->y, scroll->w, h - scroll->y);
	update_scrollbar();
	relocate_oscillators(scroll->get_position());
	synth->w = w;
	synth->h = h;
return 0;
}

SynthOscGUI* SynthWindow::add_oscillator(SynthOscillator *oscillator, int y)
{
	SynthOscGUI *result = new SynthOscGUI(this, oscillator);
	result->create_objects(y);
	return result;
}

int SynthWindow::relocate_oscillators(int position)
{
	for(int i = 0; i < synth->total_oscillators; i++)
	{
		synth->oscillators[i]->set_y(i * OSCILLATORHEIGHT - position);
	}
return 0;
}

int SynthWindow::update_scrollbar()
{
	scroll->set_position(synth->oscillator_height(), scroll->get_position(), subwindow->get_h());
return 0;
}




SynthCanvasZoomin::SynthCanvasZoomin(SynthWindow *window, int x, int y)
: BC_DownTriangleButton(x, y, 20, 20)
{ this->window = window; }
SynthCanvasZoomin::~SynthCanvasZoomin()
{
}

int SynthCanvasZoomin::handle_event()
{
	window->canvas_zoom *= 2;
	window->canvas->update();
return 0;
}



SynthCanvasZoomout::SynthCanvasZoomout(SynthWindow *window, int x, int y)
 : BC_UpTriangleButton(x, y, 20, 20)
{ this->window = window; }
SynthCanvasZoomout::~SynthCanvasZoomout()
{
}

int SynthCanvasZoomout::handle_event()
{
	window->canvas_zoom /= 2;
	window->canvas->update();
return 0;
}




SynthWaveForm::SynthWaveForm(Synth *synth, int x, int y, char *text)
 : BC_PopupMenu(x, y, 100, text)
{
	this->synth = synth;
}

SynthWaveForm::~SynthWaveForm()
{
}

int SynthWaveForm::add_items()
{
	add_item(new SynthWaveFormItem(synth, "Sine", SINE));
	add_item(new SynthWaveFormItem(synth, "Sawtooth", SAWTOOTH));
	add_item(new SynthWaveFormItem(synth, "Square", SQUARE));
	add_item(new SynthWaveFormItem(synth, "Triangle", TRIANGLE));
	add_item(new SynthWaveFormItem(synth, "Pulse", PULSE));
	add_item(new SynthWaveFormItem(synth, "Noise", NOISE));
return 0;
}

SynthWaveFormItem::SynthWaveFormItem(Synth *synth, char *text, int value)
 : BC_PopupItem(text)
{
	this->synth = synth;
	this->value = value;
}

SynthWaveFormItem::~SynthWaveFormItem()
{
}

int SynthWaveFormItem::handle_event()
{
	synth->wavefunction = value;
	update_menu();
	synth->thread->window->canvas->update();
	synth->send_configure_change();
return 0;
}













SynthCanvas::SynthCanvas(Synth *synth, SynthWindow *window, int x, int y, int w, int h)
 : BC_Canvas(x, y, w, h, BLACK)
{
	this->synth = synth;
	this->window = window;
}

SynthCanvas::~SynthCanvas()
{
}

int SynthCanvas::update()
{
	int y1, y2, y = 0;
	
	clear_box(0, 0, w, h);
	set_color(RED);

	//for(y1 = 0; y1 * window->canvas_zoom < h; y1 += h / 2)
	//{
	//	y = (int)(y1 * window->canvas_zoom);
		draw_line(0, h/2 + y, w, h/2 + y);
	//	draw_line(0, h/2 - y, w, h/2 - y);
	//}

	set_color(GREEN);
	double normalize_constant = 1 / synth->get_total_power();
	y1 = (int)(synth->get_point(0, normalize_constant) * h / 2);
	
	for(int i = 1; i < w; i++)
	{
		y2 = (int)(synth->get_point((float)i / w, normalize_constant) * h / 2 * window->canvas_zoom);
		draw_line(i - 1, h/2 - y1, i, h/2 - y2);
		y1 = y2;
	}
	flash();
return 0;
}



SynthSubWindow::SynthSubWindow(Synth *synth, int x, int y, int w, int h)
 : BC_SubWindow(x, y, w, h, MEGREY)
{
	this->synth = synth;
}
SynthSubWindow::~SynthSubWindow()
{
}


SynthFreqPot::SynthFreqPot(Synth *synth, SynthWindow *window, int x, int y)
 : BC_QPot(x, y, 40, 40, synth->base_freq, 1, 20000, LTGREY, MEGREY)
{
	this->synth = synth;
}
SynthFreqPot::~SynthFreqPot()
{
}
int SynthFreqPot::handle_event()
{
	if(get_value() > 0 && get_value() < 30000)
	{
		synth->base_freq = get_value();
		freq_text->update(get_value());
		synth->send_configure_change();
	}
return 0;
}



SynthBaseFreq::SynthBaseFreq(Synth *synth, int x, int y)
 : BC_TextBox(x, y, 70, (int)synth->base_freq, 1)
{
	this->synth = synth;
}
SynthBaseFreq::~SynthBaseFreq()
{
}
int SynthBaseFreq::handle_event()
{
	int new_value = atol(get_text());
	
	if(new_value > 0 && new_value < 30000)
	{
		synth->base_freq = new_value;
		freq_pot->update(synth->base_freq);
		synth->send_configure_change();
	}
return 0;
}




SynthClear::SynthClear(Synth *synth, int x, int y)
 : BC_BigButton(x, y, "Clear")
{
	this->synth = synth;
}
SynthClear::~SynthClear()
{
}
int SynthClear::handle_event()
{
	synth->reset();
	synth->send_configure_change();
return 0;
}







SynthScroll::SynthScroll(Synth *synth, SynthWindow *window, int x, int y, int h)
 : BC_YScrollBar(x, y, 17, h, synth->oscillator_height(), 0, window->subwindow->get_h())
{
	this->synth = synth;
	this->window = window;
}

SynthScroll::~SynthScroll()
{
}

int SynthScroll::handle_event()
{
	window->relocate_oscillators(get_position());
return 0;
}













SynthAddOsc::SynthAddOsc(Synth *synth, SynthWindow *window, int x, int y)
 : BC_BigButton(x, y, "Add")
{
	this->synth = synth;
	this->window = window;
}

SynthAddOsc::~SynthAddOsc()
{
}

int SynthAddOsc::handle_event()
{
	synth->add_oscillator();
	synth->send_configure_change();
	window->update_scrollbar();
return 0;
}



SynthDelOsc::SynthDelOsc(Synth *synth, SynthWindow *window, int x, int y)
 : BC_BigButton(x, y, "Delete")
{
	this->synth = synth;
	this->window = window;
}

SynthDelOsc::~SynthDelOsc()
{
}

int SynthDelOsc::handle_event()
{
	synth->delete_oscillator();
	window->update_scrollbar();
	window->canvas->update();
	synth->send_configure_change();
return 0;
}






SynthOscGUI::SynthOscGUI(SynthWindow *window, SynthOscillator *oscillator)
{
	this->window = window;
	this->oscillator = oscillator;
}

SynthOscGUI::~SynthOscGUI()
{
	delete title;
	delete level;
	delete phase;
	delete freq;
}

int SynthOscGUI::create_objects(int view_y)
{
	char text[1024];
	sprintf(text, "%d:", oscillator->number+1);
	window->subwindow->add_tool(title = new BC_Title(10, view_y + 15, text));
	window->subwindow->add_tool(level = new SynthOscGUILevel(oscillator, view_y));
	window->subwindow->add_tool(phase = new SynthOscGUIPhase(oscillator, view_y));
	window->subwindow->add_tool(freq = new SynthOscGUIFreq(oscillator, view_y));
return 0;
}


SynthOscGUILevel::SynthOscGUILevel(SynthOscillator *oscillator, int y)
 : BC_FPot(50, y, 35, 35, oscillator->level, INFINITYGAIN, 0, PINK, RED)
{
	this->oscillator = oscillator;
}

SynthOscGUILevel::~SynthOscGUILevel()
{
}

int SynthOscGUILevel::handle_event()
{
	oscillator->level = get_value();
	oscillator->synth->thread->window->canvas->update();
	oscillator->synth->send_configure_change();
return 0;
}



SynthOscGUIPhase::SynthOscGUIPhase(SynthOscillator *oscillator, int y)
 : BC_IPot(125, y, 35, 35, oscillator->phase * 360, 0, 360, DKGREY, BLACK)
{
	this->oscillator = oscillator;
}

SynthOscGUIPhase::~SynthOscGUIPhase()
{
}

int SynthOscGUIPhase::handle_event()
{
	oscillator->phase = (float)get_value() / 360;
	oscillator->synth->thread->window->canvas->update();
	oscillator->synth->send_configure_change();
return 0;
}



SynthOscGUIFreq::SynthOscGUIFreq(SynthOscillator *oscillator, int y)
 : BC_IPot(200, y, 35, 35, oscillator->freq_factor, 1, 100, LTGREY, MEGREY)
{
	this->oscillator = oscillator;
}

SynthOscGUIFreq::~SynthOscGUIFreq()
{
}

int SynthOscGUIFreq::handle_event()
{
	oscillator->freq_factor = get_value();
	oscillator->synth->thread->window->canvas->update();
	oscillator->synth->send_configure_change();
return 0;
}
