#include <string.h>
#include "assets.h"
#include "bezierauto.h"
#include "bezierautos.h"
#include "cache.h"
#include "file.h"
#include "mainwindow.h"
#include "patch.h"
#include "preferences.h"
#include "trackcanvas.h"
#include "tracks.h"
#include "vedit.h"
#include "vedits.h"
#include "vtrack.h"

VEdit::VEdit(MainWindow *mwindow, VEdits *edits)
 : Edit(mwindow, (Edits*)edits)
{
	this->vedits = edits;
	layer = 0;
	center_x = center_y = layer = 0;
	center_z = 1;
// 	translation_selected = 0;
// 	translation_zoom = 0;
}

VEdit::~VEdit() { }

int VEdit::load_properties_derived(FileHTAL *htal)
{
	length = htal->tag.get_property("FRAMES", (long)0);
	center_x = htal->tag.get_property("CENTERX", (long)0);
	center_y = htal->tag.get_property("CENTERY", (long)0);
	center_z = htal->tag.get_property("CENTERZ", (float)0);
	layer = htal->tag.get_property("LAYER", (long)0);
return 0;
}

// ================================================== drawing

int VEdit::draw(int flash, 
			int center_pixel, 
			int x, 
			int w, 
			int y, 
			int h, 
			int set_index_file)
{
	if(transition)
	{
// Edit is transition
		draw_transition(flash, center_pixel, x, w, y, h, set_index_file);
	}

	if(!asset) return 0;
	if(asset->silence) return 0;
	if(!edits->track->get_patch_of()->draw) return 0;

	int vertical = mwindow->tracks_vertical;

// frame start of view
	float view_start = toframes(mwindow->view_start 
							+ (vertical ? y : x) 
							* mwindow->zoom_sample, 
							mwindow->sample_rate, 
							mwindow->frame_rate);

// number of frames in the view
	float view_units = toframes((vertical ? h : w) * mwindow->zoom_sample, mwindow->sample_rate, mwindow->frame_rate);
	if(view_start + view_units < startproject) return 0;

// number of frames per pixel
	float zoom_units = toframes(mwindow->zoom_sample, mwindow->sample_rate, mwindow->frame_rate);

	float frame_w, frame_h, picture_zoom, x_zoom, y_zoom;
	get_frame_dimensions(frame_w, frame_h, picture_zoom, x_zoom, y_zoom);
// columns per picture
	float picture_columns = picture_zoom * (vertical ? frame_h * y_zoom : frame_w * x_zoom);
// number of frames in a picture
	float frames_per_picture = zoom_units * picture_columns;

// need frames_per_picture to test this
	if(startproject + length < view_start - frames_per_picture) return 0;

// size of picture
	int picture_w = (int)(picture_zoom * frame_w * x_zoom);
	int picture_h = (int)(picture_zoom * frame_h * y_zoom);

// starting frame to draw
	float frame1project;
    Edit *last_drawable = previous;

// Causes gaps when the previous frame doesn't take up a full picture
// Get the last edit which was drawable
// 	for(last_drawable = previous; 
//     	last_drawable && 
//     	((last_drawable->asset && last_drawable->asset->silence) ||
//         (last_drawable->transition)); last_drawable = last_drawable->previous)
//         ;

// The previous edit is far away.  Get first complete frame of this edit
	if((last_drawable && 
			((startproject - last_drawable->startproject - last_drawable->length >= frames_per_picture) ||
			(last_drawable->asset && last_drawable->asset->silence) ||
			(last_drawable->transition))) ||
		!last_drawable)
	{
		if(view_start > startproject)
		{
			if(frames_per_picture > 1)
				frame1project = startproject + frames_per_picture * (int)((view_start - startproject) / frames_per_picture);
			else
				frame1project = (float)((int)view_start);
		}
		else
			frame1project = startproject;
	}
	else
	{
// Get first complete frame relative to the project if there is a previous edit in the way
		if(view_start > startproject)
		{
// View starts after start of edit
			if(frames_per_picture > 1)
// Skip frames between pictures
				frame1project = frames_per_picture * (long)(view_start / frames_per_picture);
			else
// Draw every frame
				frame1project = (float)((long)view_start);
		}
		else
		{
			if(frames_per_picture > 1)
// View starts before start of edit
			{
				if(startproject / frames_per_picture == (long)(startproject / frames_per_picture))
					frame1project = (float)((long)(startproject / frames_per_picture) * frames_per_picture);
				else
					frame1project = (float)((long)(startproject / frames_per_picture + 1) * frames_per_picture);
			}
			else
				frame1project = (float)startproject;
		}
	}

// Get equivalent of starting frame in source.
	float frame1source = frame1project - startproject + startsource;

// Get ending frame to draw.
	float frame2project = startproject + length;
	if(frame2project > view_start + view_units)
	{
		frame2project = view_start + view_units;
	}

// first column of first picture
	float projectframe = frame1project;
	float sourceframe = frame1source;
	int column;
	int row = center_pixel - mwindow->zoom_track / 2;
	float step = frames_per_picture > 1 ? frames_per_picture : 1;

	mwindow->tracks->canvas->set_color(WHITE);

	File *source;
	if(asset && !(source = mwindow->cache->check_out(asset)))
	{
		printf("VEdit::draw: Couldn't check out %s for drawing.\n", asset->path);
		return 1;
	}

	float in_x1, in_y1, in_x2, in_y2;         // bounding box for asset and output
	float out_x1, out_y1, out_x2, out_y2;
	BezierAuto *before[4], *after[4];       // for bounding box
	for(int i = 0; i < 4; i++) { before[i] = 0;  after[i] = 0; }

	int result;        // to determine if frame is visible
	while(projectframe < frame2project)
	{
// Get column of frame
		column = (int)((projectframe - view_start) / zoom_units) + (vertical ? y : x);

		result = get_bounding_box(in_x1, in_y1, in_x2, in_y2,
						out_x1, out_y1, out_x2, out_y2, 
						projectframe, picture_zoom, 
						mwindow->tracks->show_output,
						before, after);

		if(!result && sourceframe >= 0)
		{
// offset the output coords
			out_x1 *= x_zoom;
			out_x2 *= x_zoom;
			out_y1 *= y_zoom;
			out_y2 *= y_zoom;
			out_x1 += vertical ? row : column;
			out_x2 += vertical ? row : column;
			out_y1 += vertical ? column : row;
			out_y2 += vertical ? column : row;

			source->set_video_position(sourceframe, mwindow->frame_rate);
			source->set_layer(layer);
// draw it on the canvas
			if(out_x2 > out_x1 + 1 && out_y2 > out_y1 + 1 && in_x2 > in_x1 + 1 && in_y2 > in_y1 + 1)
				mwindow->tracks->canvas->draw_bitmap(
					source->read_frame(mwindow->preferences->video_use_alpha, mwindow->preferences->video_floatingpoint),
					in_x1, in_y1, in_x2, in_y2, out_x1, out_y1, out_x2, out_y2, 
					mwindow->preferences->video_use_alpha);
		}

// Draw outline
		if(vertical)
			mwindow->tracks->canvas->draw_rectangle(row, column, picture_w, picture_h);
		else
			mwindow->tracks->canvas->draw_rectangle(column, row, picture_w, picture_h);

		projectframe += step;
		sourceframe += step;
	}

	mwindow->cache->check_in(asset);

// flash just this edit
	if(flash) 
	{
		int column1 = (long)((frame1project - view_start) / zoom_units) + (vertical ? y : x);
		int column2 = (long)((frame2project - view_start) / zoom_units) + (vertical ? y : x);
		if(column1 < (vertical ? y : x)) column1 = vertical ? y : x;
		if(column2 > (vertical ? y+h : x+w)) column2 = vertical ? y+h : x+w;

		if(vertical) 
		mwindow->tracks->canvas->flash(row, column1, mwindow->zoom_track, column2 - column1);
		else
		mwindow->tracks->canvas->flash(column1, row, column2 - column1, mwindow->zoom_track);
	}
return 0;
}

int VEdit::get_bounding_box(float &in_x1, float &in_y1, float &in_x2, float &in_y2,
						float &out_x1, float &out_y1, float &out_x2, float &out_y2, 
						long frame, float picture_zoom, int do_output,
						BezierAuto **before, BezierAuto **after, int autos_on)
{
	if(!asset) return 0;
// get boxes for drawing in track view
// return 1 if not visible
	float camera_z = center_z;
	float camera_x = asset->width / 2;
	float camera_y = asset->height / 2;

	float zz, xx, yy;                 // for getting parameters from get_center
	float z[6], x[6], y[6];        // camera, and output coords
	float dummy;                   // dummy for getting z from get_center

// get camera center in asset
	if(autos_on) vedits->vtrack->camera_autos->get_center(xx, yy, zz, (float)frame, 0, &before[0], &after[0]);
	else 
	{
		xx = 0;
		yy = 0;
		zz = 1;
	}

//printf("VEdit::get_bounding_box xx %f yy %f zz %f\n", xx, yy, zz);
	camera_z *= zz;
	camera_x += (xx + center_x);
	camera_y += (yy + center_y);

// get camera coords on asset
	x[0] = camera_x - (float)mwindow->track_w / 2 / camera_z;
	y[0] = camera_y - (float)mwindow->track_h / 2 / camera_z;
	x[1] = x[0] + (float)mwindow->track_w / camera_z;
	y[1] = y[0] + (float)mwindow->track_h / camera_z;

// get asset coords on camera
	x[2] = 0;
	y[2] = 0;
	x[3] = mwindow->track_w;
	y[3] = mwindow->track_h;

// crop asset coords on camera
	if(x[0] < 0)
	{
		x[2] -= x[0] * camera_z;
		x[0] = 0;
	}
	if(y[0] < 0)
	{
		y[2] -= y[0] * camera_z;
		y[0] = 0;
	}
	if(x[1] > asset->width)
	{
		x[3] -= (x[1] - asset->width) * camera_z;
		x[1] = asset->width;
	}
	if(y[1] > asset->height)
	{
		y[3] -= (y[1] - asset->height) * camera_z;
		y[1] = asset->height;
	}

// get output bounding box
	if(do_output)
	{
// get center of track on output
		if(autos_on) vedits->vtrack->projector_autos->get_center(xx, yy, zz, (float)frame, 0, &before[2], &after[2]);
		else
		{
			xx = 0;
			yy = 0;
			zz = 1;
		}

		xx += mwindow->output_w / 2;
		yy += mwindow->output_h / 2;

// get output coords on the projector
		x[4] = xx - (mwindow->track_w / 2 - x[2]) * zz;
		y[4] = yy - (mwindow->track_h / 2 - y[2]) * zz;
		x[5] = xx + (x[3] - mwindow->track_w / 2) * zz;
		y[5] = yy + (y[3] - mwindow->track_h / 2) * zz;
		
// factor projector zoom into camera zoom
		camera_z *= zz;

// crop the asset bounding box using the track bounding box
		if(x[4] < 0)
		{
			x[0] -= x[4] / camera_z;
			x[4] = 0;
		}
		if(y[4] < 0)
		{
			y[0] -= y[4] / camera_z;
			y[4] = 0;
		}
		if(x[5] > mwindow->output_w)
		{
			x[1] -= (x[5] - mwindow->output_w) / camera_z;
			x[5] = mwindow->output_w;
		}
		if(y[5] > mwindow->output_h)
		{
			y[1] -= (y[5] - mwindow->output_h) / camera_z;
			y[5] = mwindow->output_h;
		}

// scale the output coords to the picture_zoom
		out_x1 = (float)(x[4] * picture_zoom);
		out_y1 = (float)(y[4] * picture_zoom);
		out_x2 = (float)(x[5] * picture_zoom);
		out_y2 = (float)(y[5] * picture_zoom);
	}
	else
	{
		out_x1 = (float)(x[2] * picture_zoom);
		out_y1 = (float)(y[2] * picture_zoom);
		out_x2 = (float)(x[3] * picture_zoom);
		out_y2 = (float)(y[3] * picture_zoom);
	}

// get input coords
	in_x1 = (float)x[0];
	in_y1 = (float)y[0];
	in_x2 = (float)x[1];
	in_y2 = (float)y[1];

	if(out_x2 <= out_x1 || out_y2 <= out_y1 || in_x2 <= in_x1 || in_y2 <= in_y1)
		return 1;
	else
		return 0;
return 0;
}

int VEdit::get_handle_parameters(long &left, long &right, long &left_sample, long &right_sample, float view_start, float zoom_units)
{
	left = (long)((startproject - view_start) / zoom_units);
	right = (long)(((startproject + length) - view_start) / zoom_units);

// want samples for proper cursor positioning
	left_sample = tosamples(startproject, mwindow->sample_rate, mwindow->frame_rate);
	right_sample = tosamples(startproject + length, mwindow->sample_rate, mwindow->frame_rate);
return 0;
}

int VEdit::get_frame_dimensions(float &frame_w, float &frame_h, float &picture_zoom, float &x_zoom, float &y_zoom)
{
// real width of rendered frame
	frame_w = mwindow->tracks->show_output ? mwindow->output_w : mwindow->track_w;
// real height of rendered frame
	frame_h = mwindow->tracks->show_output ? mwindow->output_h : mwindow->track_h;
// size of picture relative to size of real rendered frame
	picture_zoom = (float)mwindow->zoom_track / (mwindow->tracks_vertical ? frame_w : (frame_w / mwindow->get_aspect_ratio()));
	x_zoom = 1;
	y_zoom = (float)frame_w / mwindow->get_aspect_ratio() / frame_h;
return 0;
}

// ================================================== editing

int VEdit::select_translation(int cursor_x, int cursor_y, float view_start, float zoom_units)
{
	long left = (long)((startproject - view_start) / zoom_units);
	long right = (long)(((startproject + length) - view_start) / zoom_units);

	if(cursor_x > left && cursor_x < right)
	{
		vedits->vtrack->translation_selected = 1;
		vedits->vtrack->translation_zoom = 0;
		if(mwindow->tracks_vertical)
		{
			cursor_x ^= cursor_y;
			cursor_y ^= cursor_x;
			cursor_x ^= cursor_y;
		}

// cursor position is now relative to frame
		float frame_w, frame_h, picture_zoom, x_zoom, y_zoom;
		get_frame_dimensions(frame_w, frame_h, picture_zoom, x_zoom, y_zoom);
		vedits->vtrack->get_virtual_center(this, cursor_x, cursor_y, frame_w, frame_h, picture_zoom);
		return 1;
	}
	else
	{
		vedits->vtrack->translation_selected = 0;
		return 0;
	}
return 0;
}

int VEdit::update_translation(int cursor_x, int cursor_y, int shift_down, float view_start, float zoom_units)
{
	if(vedits->vtrack->translation_selected && asset && !asset->silence)
	{
		if(mwindow->tracks_vertical)
		{
			cursor_x ^= cursor_y;
			cursor_y ^= cursor_x;
			cursor_x ^= cursor_y;
		}

// cursor position is now relative to frame
		float frame_w, frame_h, picture_zoom, x_zoom, y_zoom;
		get_frame_dimensions(frame_w, frame_h, picture_zoom, x_zoom, y_zoom);

		int need_redraw = 0;
		if(shift_down)
		{
			if(!vedits->vtrack->translation_zoom)
			{
				vedits->vtrack->translation_zoom = 1;
				vedits->vtrack->get_virtual_center(this, cursor_x, cursor_y, frame_w, frame_h, picture_zoom);
			}

			float new_center_z = (float)(cursor_y - vedits->vtrack->virtual_center_y) / (frame_w / mwindow->get_aspect_ratio());
			if(new_center_z != center_z)
			{
				center_z = new_center_z;
				need_redraw = 1;
			}
		}
		else
		{
			if(vedits->vtrack->translation_zoom)
			{
				vedits->vtrack->translation_zoom = 0;
				vedits->vtrack->get_virtual_center(this, cursor_x, cursor_y, frame_w, frame_h, picture_zoom);
			}

			int new_x = (int)((cursor_x - vedits->vtrack->virtual_center_x) / center_z);
			int new_y = (int)((cursor_y - vedits->vtrack->virtual_center_y) / center_z);
			if(new_x != center_x || new_y != center_y)
			{
				center_x = new_x;
				center_y = new_y;
				need_redraw = 1;
			}
		}

		if(need_redraw)
		{
			mwindow->tracks->hide_overlays(0);
			long left = (long)((startproject - view_start) / zoom_units);
			if(left < 0) left = 0;
			long pixels = (int)(length / zoom_units);
			float frame_pixels = picture_zoom * (mwindow->tracks_vertical ? y_zoom * frame_h : x_zoom * frame_w);
			pixels = (long)(pixels / frame_pixels);
			pixels = (long)(pixels * frame_pixels);
			pixels = (long)(pixels + frame_pixels);
			if(pixels + left > mwindow->tracks->view_pixels()) 
				pixels = mwindow->tracks->view_pixels();

			if(mwindow->tracks_vertical)
				mwindow->tracks->canvas->clear_box(vedits->vtrack->pixel, 
													left,
													mwindow->zoom_track,
													pixels);
			else
				mwindow->tracks->canvas->clear_box(left,
													vedits->vtrack->pixel, 
													pixels,
													mwindow->zoom_track);

			draw(0, 
				vedits->track->pixel + mwindow->zoom_track / 2, 
				0, 
				mwindow->tracks->canvas->w, 
				0, 
				mwindow->tracks->canvas->h, 
				0);
			mwindow->tracks->show_overlays(1);
		}
		return 1;
	}
	return 0;
return 0;
}

int VEdit::reset_translation(long start, long end)
{
//	if(startproject + length < start || startproject > end)
//		return 0;    // nothing selected

// Resets all the translation on the track
	center_x = 0;
	center_y = 0;
	center_z = 1;
	return 1;
return 0;
}

int VEdit::end_translation()
{
	if(vedits->vtrack->translation_selected)
	{
		vedits->vtrack->translation_selected = 0;
		return 1;
	}
	return 0;
return 0;
}

int VEdit::render(VFrame **output, 
					PluginBuffer *output_buffer,
					long output_byte_offset, 
					long input_len, 
					long input_position, 
					float step, 
					BezierAuto **before, 
					BezierAuto **after,
					int autos_on)
{
	float in_x1, in_y1, in_x2, in_y2;
	float out_x1, out_y1, out_x2, out_y2;
	int alpha = VMAX;    // amount of opacity for feathering
	long output_position = 0;    // frame in output buffer

	if(!asset) return 0;    // Transition
	if(asset->silence) return 0;       // Silence
	if(input_len <= 0) return 0;

// get the source file
	File *source;
	VFrame *frame;

	if(!(source = mwindow->cache->check_out(asset)))
	{
		printf("VEdit::render Couldn't open %s.\n", asset->path);
		return 1;
	}

	alpha = VMAX;

//	if(input_position >= startproject + length && input_position <= startproject + length + feather_right)
	if(input_position >= startproject + length && input_position < startproject + length + feather_right)
	{
// in right feather area
		alpha = (int)(VMAX * (float)(feather_right - (input_position - startproject - length)) / feather_right);
	}

// read frame and overlay
	get_bounding_box(in_x1, in_y1, in_x2, in_y2,
					out_x1, out_y1, out_x2, out_y2,
					input_position, 1, 0, before, after, autos_on);

	source->set_video_position(input_position - startproject + startsource, mwindow->frame_rate);
	source->set_layer(layer);
	if(in_x2 > in_x1 && in_y2 > in_y1 && out_x2 > out_x1 && out_y2 > out_y1)
	{
		source->read_frame(output[0], output_buffer, output_byte_offset, 
					in_x1, in_y1, in_x2, in_y2,
					out_x1, out_y1, out_x2, out_y2, alpha,
					mwindow->preferences->video_use_alpha, mwindow->preferences->video_floatingpoint, 
					mwindow->preferences->video_interpolate);
	}

	output_position++;
	mwindow->cache->check_in(asset);
return 0;
}

long VEdit::compressed_frame_size(long render_video_position)
{
	File *source;
	long result;

	if(!(source = mwindow->cache->check_out(asset)))
	{
		printf("VEdit::compressed_frame_size Couldn't open %s.\n", asset->path);
		return 0;
	}

	source->set_video_position(render_video_position - startproject + startsource, mwindow->frame_rate);
	source->set_layer(layer);
	result = source->compressed_frame_size();
	mwindow->cache->check_in(asset);
	return result;
}

int VEdit::read_compressed_frame(VFrame *frame, long project_position)
{
	int result;
	File *source;

	if(!(source = mwindow->cache->check_out(asset)))
	{
		printf("VEdit::read_compressed_frame: Couldn't open %s.\n", asset->path);
		return 0;
	}

	source->set_video_position(project_position - startproject + startsource, mwindow->frame_rate);
	source->set_layer(layer);
	result = source->read_compressed_frame(frame);
	mwindow->cache->check_in(asset);
	return result;
return 0;
}

int VEdit::read_raw_frame(VFrame *frame, 
				PluginBuffer *output_buffer, 
				long output_byte_offset, 
				long project_position)
{
	int result;
	File *source;

	if(!(source = mwindow->cache->check_out(asset)))
	{
		printf("VEdit::read_compressed_frame: Couldn't open %s.\n", asset->path);
		return 0;
	}

	source->set_video_position(project_position - startproject + startsource, mwindow->frame_rate);
	source->set_layer(layer);
	result = source->read_raw_frame(frame, output_buffer, output_byte_offset);
	mwindow->cache->check_in(asset);
	return result;
return 0;
}

int VEdit::copy_properties_derived(FileHTAL *htal, long length_in_selection)
{
	htal->tag.set_property("FRAMES", length_in_selection);
	htal->tag.set_property("CENTERX", center_x);
	htal->tag.set_property("CENTERY", center_y);
	htal->tag.set_property("CENTERZ", center_z);
	htal->tag.set_property("LAYER", layer);
return 0;
}

int VEdit::dump_derived()
{
	printf("	VEdit::dump_derived\n");
	printf("		startproject %ld\n", startproject);
	printf("		length %ld\n", length);
	printf("		center_x %d\n", center_x);
	printf("		center_y %d\n", center_y);
return 0;
}

long VEdit::get_source_end()
{
	if(!asset || asset->silence) return -1;   // Infinity

	File source;
	if(source.try_to_open_file(mwindow->plugindb, asset, 1, 0))
	{
// couldn't open source file
		printf("Edits::modify_handles Couldn't open %s.\n", asset->path);
		return -1;
	}

	long endsource = source.get_video_length(mwindow->frame_rate);
	source.close_file();
	return endsource;
}
