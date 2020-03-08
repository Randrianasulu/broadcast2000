#ifndef RECORDTRANSPORT_H
#define RECORDTRANSPORT_H

#include "bcbase.h"
#include "record.inc"
#include "recordengine.inc"

class RecordGUIDuplex;
class RecordGUIEnd;
class RecordGUIBack;
class RecordGUIFwd;
class RecordGUIPlay;
class RecordGUIRec;
class RecordGUIStop;
class RecordGUIRewind;
class RecordGUIRecFrame;

class RecordGUITransport
{
public:
	RecordGUITransport(Record *record, RecordEngine *engine, BC_WindowBase *window);
	~RecordGUITransport();

	int create_objects(int x, int y, int h);
	int keypress_event();

	BC_WindowBase *window;
	Record *record;
	RecordGUIDuplex *duplex_button;
	RecordGUIEnd *end_button;
	RecordGUIFwd *fwd_button;
	RecordGUIBack *back_button;
	RecordGUIRewind *rewind_button;
	RecordGUIStop *stop_button;
	RecordGUIPlay *play_button;
	RecordGUIRec *record_button;
	RecordGUIRecFrame *record_frame;
	RecordEngine *engine;
	int x_end;
};



class RecordGUIRec : public BC_RecButton
{
public:
	RecordGUIRec(RecordEngine *engine, int x, int y, int h);
	~RecordGUIRec();

	int handle_event();
	int keypress_event();
	RecordEngine *engine;
};

class RecordGUIRecFrame : public BC_FrameRecButton
{
public:
	RecordGUIRecFrame(RecordEngine *engine, int x, int y, int h);
	~RecordGUIRecFrame();

	int handle_event();
	int keypress_event();
	RecordEngine *engine;
};

class RecordGUIPlay : public BC_ForwardButton
{
public:
	RecordGUIPlay(RecordEngine *engine, int x, int y, int h);
	~RecordGUIPlay();

	int handle_event();
	int keypress_event();
	RecordEngine *engine;
};

class RecordGUIStop : public BC_StopButton
{
public:
	RecordGUIStop(RecordEngine *engine, int x, int y, int h);
	~RecordGUIStop();

	int handle_event();
	int keypress_event();
	RecordEngine *engine;
};

class RecordGUIRewind : public BC_RewindButton
{
public:
	RecordGUIRewind(Record *record, RecordEngine *engine, int x, int y, int h);
	~RecordGUIRewind();

	int handle_event();
	int keypress_event();
	RecordEngine *engine;
	Record *record;
};

class RecordGUIBack : public BC_FastReverseButton
{
public:
	RecordGUIBack(Record *record, RecordEngine *engine, int x, int y, int h);
	~RecordGUIBack();

	int handle_event();
	int button_press();
	int button_release();
	int repeat();
	int keypress_event();
	long count;

	RecordEngine *engine;
	Record *record;
};

class RecordGUIFwd : public BC_FastForwardButton
{
public:
	RecordGUIFwd(Record *record, RecordEngine *engine, int x, int y, int h);
	~RecordGUIFwd();

	int handle_event();
	int button_press();
	int button_release();
	int repeat();
	int keypress_event();

	long count;
	RecordEngine *engine;
	Record *record;
};

class RecordGUIEnd : public BC_EndButton
{
public:
	RecordGUIEnd(Record *record, RecordEngine *engine, int x, int y, int h);
	~RecordGUIEnd();

	int handle_event();
	int keypress_event();
	RecordEngine *engine;
	Record *record;
};

class RecordGUIDuplex : public BC_DuplexButton
{
public:
	RecordGUIDuplex(RecordEngine *engine, int x, int y, int h);
	~RecordGUIDuplex();

	int handle_event();
	RecordEngine *engine;
};

#endif
