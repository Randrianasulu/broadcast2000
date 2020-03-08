#include <string.h>
#include "labels.h"
#include "mainsamplescroll.h"
#include "mainwindow.h"
#include "mainwindowgui.h"
#include "statusbar.h"
#include "timebar.h"
#include "tracks.h"



int MainWindow::expand_sample()
{
	if(gui)
	{
		if(zoom_sample < 2000000)
		{
			zoom_sample *= 2;
			find_cursor();
			timebar->samplemovement();
			draw();
		}
	}
return 0;
}

int MainWindow::zoom_in_sample()
{
	if(gui)
	{
		if(zoom_sample > 1)
		{
			zoom_sample /= 2;
			find_cursor();
			timebar->samplemovement();
			draw();
		}
	}
return 0;
}

int MainWindow::find_cursor()
{
	if(playback_cursor_visible /*&& selectionend == selectionstart */)
		view_start = last_playback_position - tracks->view_samples() / 4;
	else
		view_start = selectionend / 2 + selectionstart / 2 - tracks->view_samples() / 2;

	if(view_start < 0) view_start = 0;
	// scrollbar is set during draw
return 0;
}

int MainWindow::fit_sample()
{
	if(gui)
	{
		if(selectionstart == selectionend)
		{
			long longest_ = tracks->total_samples();

			for(zoom_sample = 1; tracks->view_samples() < longest_; zoom_sample *= 2)
				;                       // find the optimum zoom based on the entire window size

			view_start = 0;         // set the position to 0
			timebar->samplemovement();
			draw();                   // draw everything over and set scrollbar
		}
		else
		{
			long length = selectionend - selectionstart;

			for(zoom_sample = 1; zoom_sample * (long)tracks->view_pixels() <= length; zoom_sample *= 2)
				;                       // find the optimum zoom based on the entire window size

			find_cursor();
			timebar->samplemovement();
			draw();                   // draw everything over and set scrollbar
		}
	}
return 0;
}

int MainWindow::goto_end()
{
	if(gui)
	{
		long distance;

		distance = tracks->total_samples() - view_start - tracks->view_samples() / 2;

		if(view_start + distance < 0) distance = -view_start;

		tracks->draw_cursor(0);
		if(gui->shift_down())
		{
			long current = tracks->total_samples();

			if(current < selectionstart) selectionstart = current;
			else if(current > selectionend) selectionend = current;
		}
		else
		{
			selectionstart = selectionend = tracks->total_samples();
		}
		tracks->draw_cursor(1);      // show
		samplemovement(distance);
		gui->statusbar->draw();
		timebar->draw();
	}
return 0;
}

int MainWindow::goto_start()
{
	if(gui)
	{
		long distance;

		distance = -view_start;

		tracks->draw_cursor(0);
		if(gui->shift_down())
		{
			selectionstart = 0;
		}
		else
		{
			selectionstart = selectionend = 0;
		}
		tracks->draw_cursor(1);      // show
		if(distance) samplemovement(-view_start);
		gui->statusbar->draw();
		timebar->draw();
	}
return 0;
}

int MainWindow::move_left(long distance)
{
	if(gui)
	{
		if(!distance) distance = tracks->view_samples() / 5;
		if(view_start < distance)
		{
			distance = view_start;
		}
		samplemovement(-distance);
	}
return 0;
}

int MainWindow::move_right(long distance)
{
	if(gui)
	{
		if(!distance) distance = tracks->view_samples() / 5;
		samplemovement(distance);
	}
return 0;
}

int MainWindow::next_label()
{
	Label *current;
	Labels *labels = timebar->labels;

// Test for label under cursor position
	for(current = labels->first; 
		current && !labels->equivalent(current->position, selectionend); 
		current = NEXT)
		;

// Start with first label
	if(!current) current = labels->first;

// 	if(gui->shift_down())
// 		current = timebar->labels->label_of(selectionend);
// 	else
// 		current = timebar->labels->label_of(selectionstart);

	if(current)
	{
		while(current && (current->position < selectionend || labels->equivalent(current->position, selectionend)))
			current = NEXT;

//		if(current->position <= selectionstart || gui->shift_down()) current = current->next;

		if(current)
		{
			tracks->draw_cursor(0);   // hide old cursor
			selectionend = current->position;
			if(!gui->shift_down()) selectionstart = selectionend;

			if(selectionend >= view_start + tracks->view_samples())
			{
				tracks->draw_cursor(0);   // show old cursor
				samplemovement(selectionend - view_start - tracks->view_samples() / 2);
			}
			else
				tracks->draw_cursor(1);   // show new cursor
		}
	}
return 0;
}

int MainWindow::prev_label()
{
	Label *current;
	Labels *labels = timebar->labels;

// Test for label under cursor position
	for(current = labels->first; 
		current && !labels->equivalent(current->position, selectionstart); 
		current = NEXT)
		;

// 	if(gui->shift_down())
// 		current = timebar->labels->label_of(selectionstart);
// 	else
// 		current = timebar->labels->label_of(selectionend);

// Start with last label
	if(!current) current = labels->last;

	if(current)
	{
		while(current && (current->position > selectionstart || labels->equivalent(current->position, selectionstart)))
			current = PREVIOUS;

//		if(current->position >= selectionend || gui->shift_down()) current = current->previous;

		if(current) 
		{
        	long position_change;
			tracks->draw_cursor(0);
			selectionstart = current->position;
			if(!gui->shift_down()) selectionend = selectionstart;
			
			if(selectionstart <= view_start)
			{
				tracks->draw_cursor(0);     // show old cursor
                position_change = selectionstart - view_start - tracks->view_samples() / 2;
                if(view_start + position_change < 0) position_change = -view_start;
				samplemovement(position_change);
			}
			else
				tracks->draw_cursor(1);
		}
	}
return 0;
}

int MainWindow::expand_y()
{
	if(gui)
	{
		tracks->hide_overlays(0);
		tracks->expand_y();
		gui->statusbar->draw();
		tracks->show_overlays(1);
	}
return 0;
}

int MainWindow::zoom_in_y()
{
	if(gui)
	{
		tracks->hide_overlays(0);
		tracks->zoom_y();
		gui->statusbar->draw();
		tracks->show_overlays(1);
	}
return 0;
}

int MainWindow::expand_t()
{
	if(gui)
	{
		if(zoom_track > 1)
		{
			tracks->hide_overlays(0);
			zoom_track /= 2;
			if(zoom_y > 1) zoom_y /= 2;

			tracks->expand_t();
			gui->statusbar->draw();
			tracks->show_overlays(1);
		}
	}
return 0;
}

int MainWindow::zoom_in_t()
{
	if(gui)
	{
		if(zoom_track < 65536)
		{
			tracks->hide_overlays(0);
			zoom_track *= 2;
			if(zoom_y < 65536) zoom_y *= 2;

			tracks->zoom_t();
			gui->statusbar->draw();
			tracks->show_overlays(1);
		}
	}
return 0;
}

int MainWindow::samplemovement(long distance)
{
	if(gui)
	{
// hide everything ===============
		tracks->hide_overlays(0);
		view_start += distance;
		tracks->samplemovement(distance);
// show everything ===============
		tracks->show_overlays(1);
		gui->statusbar->draw();
		timebar->samplemovement();

		gui->mainsamplescroll->set_position();
	}
return 0;
}
