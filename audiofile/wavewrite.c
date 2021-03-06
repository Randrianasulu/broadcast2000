/*
	Audio File Library
	Copyright (C) 1998-1999, Michael Pruett <michael@68k.org>

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
	wavewrite.c

	This file contains routines which facilitate writing to WAVE files.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#include "audiofile.h"
#include "afinternal.h"
#include "byteorder.h"
#include "wave.h"
#include "util.h"
#include "block.h"

int waveWriteFrames (const AFfilehandle file, int track, void *samples,
	const int count);
int waveSyncFile (AFfilehandle file);

static void WriteFormat (const AFfilehandle file);
static int WriteData (const AFfilehandle file, int track, void *samples,
	const int count);

/*
	PCM is the only format that is supported at present.
	Logarithmic encoding sucks a fat dick.
*/
static void WriteFormat (const AFfilehandle file)
{
	u_int16_t	formatTag, channelCount;
	u_int32_t	sampleRate, averageBytesPerSecond;
	u_int16_t	blockAlign;
	u_int32_t	chunkSize;

	assert(file != NULL);

	fwrite("fmt ", 4, 1, file->fp);
	chunkSize = 16;
	chunkSize = HOST_TO_LENDIAN_INT32(chunkSize);
	fwrite(&chunkSize, 4, 1, file->fp);

	formatTag = WAVE_FORMAT_PCM;
	formatTag = HOST_TO_LENDIAN_INT16(formatTag);
	fwrite(&formatTag, 2, 1, file->fp);
	formatTag = LENDIAN_TO_HOST_INT16(formatTag);

	channelCount = file->channelCount;
	channelCount = HOST_TO_LENDIAN_INT16(channelCount);
	fwrite(&channelCount, 2, 1, file->fp);

	sampleRate = file->sampleRate;
	sampleRate = HOST_TO_LENDIAN_INT32(sampleRate);
	fwrite(&sampleRate, 4, 1, file->fp);

	averageBytesPerSecond =
		file->channelCount * file->sampleRate * (file->sampleWidth / 8);
	averageBytesPerSecond = HOST_TO_LENDIAN_INT32(averageBytesPerSecond);
	fwrite(&averageBytesPerSecond, 4, 1, file->fp);

	blockAlign = file->channelCount * (file->sampleWidth / 8);
	blockAlign = HOST_TO_LENDIAN_INT16(blockAlign);
	fwrite(&blockAlign, 2, 1, file->fp);

	if (formatTag == WAVE_FORMAT_PCM)
	{
		u_int16_t	bitsPerSample;

		bitsPerSample = file->sampleWidth;
		bitsPerSample = HOST_TO_LENDIAN_INT16(bitsPerSample);
		fwrite(&bitsPerSample, 2, 1, file->fp);
	}
}

/* Again, only 8-, 16-, or 32-bit sample widths are supported. */
static int WriteData (const AFfilehandle file, int track, void *samples,
	const int count)
{
	u_int32_t	frameSize, chunkSize;

	assert(file);
	assert(samples);

	/*
		Division by 8 should be offset by addition by 7 in order to
		accomodate bit depths which are less than 8 bits.
	*/
	frameSize = file->channelCount * ((file->sampleWidth + 7) / 8);
	chunkSize = frameSize * file->frameCount;
	chunkSize = HOST_TO_LENDIAN_INT32(chunkSize);

	if (file->dataStart == 0)
	{
		fwrite("data", 4, 1, file->fp);
		fwrite(&chunkSize, 4, 1, file->fp);
		file->dataStart = ftell(file->fp);
	}

	fseek(file->fp, file->dataStart + file->currentFrame * frameSize, SEEK_SET);

	return _af_blockWriteFrames(file, track, samples, count);
}

int waveWriteFrames (const AFfilehandle file, int track, void *samples,
	const int count)
{
	int		result;

	if (file->dataStart == 0)
	{
		u_int32_t	zero = 0;

		fwrite("RIFF", 4, 1, file->fp);
		fwrite(&zero, 4, 1, file->fp);
		fwrite("WAVE", 4, 1, file->fp);

		WriteFormat(file);
	}

	result = WriteData(file, track, samples, count);

	return result;
}

int waveSyncFile (AFfilehandle file)
{
	if (file->dataStart != 0)
	{
		size_t	dataLength, fileLength;

		dataLength = file->frameCount * file->channelCount *
			((file->sampleWidth + 7) / 8);

		dataLength = HOST_TO_LENDIAN_INT32(dataLength);
		fseek(file->fp, file->dataStart - 4, SEEK_SET);
		fwrite(&dataLength, 4, 1, file->fp);

		fseek(file->fp, 0, SEEK_END);
		fileLength = ftell(file->fp);
		fileLength -= 8;
		fileLength = HOST_TO_LENDIAN_INT32(fileLength);

		fseek(file->fp, 4, SEEK_SET);
		fwrite(&fileLength, 4, 1, file->fp);
	}

	return 0;
}
