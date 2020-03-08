#include <string.h>
#include "assets.h"
#include "file.inc"
#include "errorbox.h"
#include "formatcheck.h"

FormatCheck::FormatCheck(Asset *asset)
{
	this->asset = asset;
}

FormatCheck::~FormatCheck()
{
}

int FormatCheck::check_format()
{
	int result = 0;

	if(!result && asset->video_data)
	{
// Only 1 format can store video.
		if(asset->format != MOV && asset->format != JPEG_LIST)
		{
			ErrorBox error;
			error.create_objects("The format you selected doesn't support video.");
			error.run_window();
			result = 1;
		}
	}
	
	if(!result && asset->audio_data)
	{
		if(asset->format == JPEG_LIST)
		{
			ErrorBox error;
			error.create_objects("The format you selected doesn't support audio.");
			error.run_window();
			result = 1;
		}

		if(!result && asset->bits == BITSIMA4 && asset->format != MOV)
		{
			ErrorBox error;
			error.create_objects("IMA4 compression is only available in Quicktime movies.");
			error.run_window();
			result = 1;
		}

		if(!result && asset->bits == BITSULAW && 
			asset->format != MOV &&
			asset->format != PCM)
		{
			ErrorBox error;
			error.create_objects("ULAW compression is only available in", 
				"Quicktime Movies and PCM files.");
			error.run_window();
			result = 1;
		}
	}

	return result;
return 0;
}
