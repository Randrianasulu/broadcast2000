/*
	Audio File Library
	Copyright (C) 1999, Michael Pruett <michael@68k.org>

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Library General Public
	License as published by the Free Software Foundation; either
	version 2 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Library General Public License for more details.

	You should have received a copy of the GNU Library General Public
	License along with this library; if not, write to the 
	Free Software Foundation, Inc., 59 Temple Place - Suite 330, 
	Boston, MA  02111-1307  USA.
*/

/*
	block.h

	This file contains block read and write function prototypes.
*/

#ifndef BLOCK_H
#define BLOCK_H

#include "audiofile.h"

int _af_blockReadFrames (AFfilehandle file, int track, void *samples,
	int frameCount);
int _af_blockWriteFrames (AFfilehandle file, int track, void *samples,
	int frameCount);

#endif
