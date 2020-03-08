#include <string.h>
#include "bcbase.h"
#include "libdv.h"
#include "vframe.h"

int main()
{
	FILE *fd = fopen("/root/newbc/bcast/debug", "r");
	unsigned char *buffer = (unsigned char*)calloc(1, 120000);
	dv_t *dv = dv_new();
	BC_Canvas *canvas;
	BC_Bitmap *bitmap;
	VFrame vframe(0, 720, 480, VFRAME_RGB888);
	
	fread(buffer, 120000, 1, fd);
	
	BC_Window window("Test", 720, 540, 720, 540);
	window.add_tool(canvas = new BC_Canvas(0, 0, 720, 540));
	bitmap = canvas->new_bitmap(720, 540);
	
	dv_read_frame(dv, 
		vframe.get_rows(), 
		buffer, 
		120000,
		DV_RGB888);
	
	bitmap->read_frame(vframe.get_rows(), 720, 480);
	canvas->draw_bitmap(bitmap, 0);
	window.run_window();
return 0;
}
