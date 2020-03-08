#include <string.h>
#include "auto.h"
#include "commonrender.h"
#include "commonrenderthread.h"
#include "mainwindow.h"
#include "patch.h"
#include "patchbay.h"
#include "playabletracks.h"
#include "preferences.h"
#include "renderengine.h"
#include "toggleautos.h"
#include "tracks.h"

CommonRender::CommonRender(MainWindow *mwindow, RenderEngine *renderengine)
 : Thread()
{
	this->mwindow = mwindow;
	this->renderengine = renderengine;
	current_position = 0;
	interrupt = 0;
	done = 0;
	last_playback = 0;
	thread = 0;
	asynchronous = 1;
}

CommonRender::~CommonRender()
{
}


int CommonRender::wait_for_completion()
{
	join();
return 0;
}

int CommonRender::get_boundaries(long &current_render_length)
{
	long loop_end = tounits(mwindow->loop_end);
	long loop_start = tounits(mwindow->loop_start);
	long start_position = tounits(renderengine->start_position);
	long end_position = tounits(renderengine->end_position);

// test boundaries if no loop and not infinite
	if((renderengine->speed == FRAME_SPEED) ||
		(!(renderengine->follow_loop && mwindow->loop_playback) && !renderengine->infinite))
	{
		if(!renderengine->reverse)       // forward playback
		{
			if(current_position + current_render_length >= end_position)
			{
				last_playback = 1;
				current_render_length = end_position - current_position;
			}
		}
		else               // reverse playback
		{
			if(current_position - current_render_length <= start_position)
			{
				last_playback = 1;
				current_render_length = current_position - start_position;
			}
		}
	}

// test against loop boundaries
	if(mwindow->loop_playback && renderengine->follow_loop && !renderengine->infinite)
	{
		long segment_end = renderengine->reverse ? current_position - current_render_length : current_position + current_render_length;

		if(!renderengine->reverse)
		{
			if(segment_end > loop_end)
			{
				current_render_length = loop_end - current_position;
			}
		}
		else
		{
			if(segment_end < loop_start)
			{
				current_render_length = current_position - loop_start;
			}
		}
	}
return 0;
}

int CommonRender::test_virtualnodes(long current_position, long &current_input_length, int data_type, int reverse)
{
	int result = 0;
	long nearest_auto, longest_duration;
	Track* current_track;
	Patch* current_patch;
	Auto* current_auto;


// Test playback status against virtual console.
	for(current_track = mwindow->tracks->first, current_patch = mwindow->patches->first;
		current_track && !result; 
		current_track = current_track->next, current_patch = current_patch->next)
	{
		if(current_track->data_type == data_type)
		{
			if(thread->playable_tracks->is_playable(current_track, current_patch, current_position, renderengine->reverse))
			{
				if(!thread->playable_tracks->is_listed(current_track))
					result = 1;
			}
			else
			if(thread->playable_tracks->is_listed(current_track))
				result = 1;
		}
	}

// Test transitions against virtual console
	for(current_track = mwindow->tracks->first; 
		current_track && !result; 
		current_track = current_track->next)
	{
		if(current_track->data_type == data_type) result = current_track->test_transition(current_position);
	}
// Don't clip input length if only rendering 1
	if(current_input_length == 1)  return result;

// Get length of time until next playback change
	current_track = mwindow->tracks->first;
	current_patch = mwindow->patches->first;
	if(reverse)
	{
// Reverse playback
		nearest_auto = current_position - current_input_length;

		while(current_track)
		{
			if(current_patch->automate && current_patch->play && current_track->data_type == data_type)
			{
				current_auto = current_track->play_autos->nearest_before(current_position);
				if(current_auto && nearest_auto < current_auto->position) nearest_auto = current_auto->position;
			}

			current_track = current_track->next;
			current_patch = current_patch->next;
		}

		if(current_position - nearest_auto < current_input_length)
		{
			current_input_length = current_position - nearest_auto;
		}
	}
	else
	{
// Forward playback
		nearest_auto = current_position + current_input_length;

		while(current_track)
		{
			if(current_patch->automate && current_patch->play && current_track->data_type == data_type)
			{
				current_auto = current_track->play_autos->nearest_after(current_position);
				if(current_auto && nearest_auto > current_auto->position) nearest_auto = current_auto->position;
			}

			current_track = current_track->next;
			current_patch = current_patch->next;
		}

		if(nearest_auto - current_position < current_input_length)
		{
			current_input_length = nearest_auto - current_position;
		}
	}

// Get length of time until next transition and edit change
	for(current_track = mwindow->tracks->first;
		 current_track; 
		 current_track = current_track->next)
	{
		if(current_track->data_type == data_type)
		{
			longest_duration = current_track->edit_change_duration(current_position, current_input_length, reverse, 1);
			if(longest_duration < current_input_length) current_input_length = longest_duration;

			if(mwindow->preferences->test_playback_edits)
			{
				longest_duration = current_track->edit_change_duration(current_position, current_input_length, reverse, 0);
				if(longest_duration < current_input_length) current_input_length = longest_duration;
			}
		}
	}

//printf("ARender:: result %d length %ld\n", result, current_input_length);
	return result;
return 0;
}


int CommonRender::advance_position(long current_render_length)
{
	long loop_end = tounits(mwindow->loop_end);
	long loop_start = tounits(mwindow->loop_start);

// advance the playback position
	if(renderengine->reverse)
		current_position -= current_render_length;
	else
		current_position += current_render_length;

// test loop again
	if(mwindow->loop_playback && renderengine->follow_loop && !renderengine->infinite)
	{
		if(renderengine->reverse)
		{
			if(current_position <= loop_start)
				current_position = loop_end;
		}
		else
		{
			if(current_position >= loop_end)
				current_position = loop_start;
		}
	}
return 0;
}

long int CommonRender::tounits(long position)
{
	return position;
}

long int CommonRender::fromunits(long position)
{
	return position;
}
