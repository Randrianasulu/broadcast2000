#ifndef BCPROGRESS_H
#define BCPROGRESS_H

#include "bctool.h"

class BC_ProgressBar : public BC_Tool
{
public:
	BC_ProgressBar(int x_, int y_, int w_, int h_, long length_);
	int create_tool_objects();

	int draw();
	int update(long position_);
	int update_length(long length);
	int stop_progress();

	long length, position;
	int percentage;
};

#endif
