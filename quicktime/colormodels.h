#ifndef COLORMODELS_H
#define COLORMODELS_H

// Colormodels
#define BC_TRANSPARENCY 0
#define BC_COMPRESSED   1
#define BC_RGB8         2
#define BC_RGB565       3
#define BC_BGR565       4
#define BC_BGR888       5
#define BC_BGR8888      6
// Working bitmaps are packed to simplify processing
#define BC_RGB888       9
#define BC_RGBA8888     10
#define BC_RGB161616    11
#define BC_RGBA16161616 12
#define BC_YUV888       13
#define BC_YUVA8888     14
#define BC_YUV161616    15
#define BC_YUVA16161616 16
// Planar
#define BC_YUV420P      7
#define BC_YUV422P      17
#define BC_YUV422       19
#define BC_YUV411P      18

// For communication with the X Server
#define FOURCC_YV12 0x32315659  /* YV12   YUV420P */
#define FOURCC_YUV2 0x32595559  /* YUV2   YUV422 */
#define FOURCC_I420 0x30323449  /* I420   Intel Indeo 4 */

#undef RECLIP
#define RECLIP(x, y, z) ((x) = ((x) < (y) ? (y) : ((x) > (z) ? (z) : (x))))

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	long rtoy_tab[256], gtoy_tab[256], btoy_tab[256];
	long rtou_tab[256], gtou_tab[256], btou_tab[256];
	long rtov_tab[256], gtov_tab[256], btov_tab[256];

	long vtor_tab[256], vtog_tab[256];
	long utog_tab[256], utob_tab[256];
	long *vtor, *vtog, *utog, *utob;
} cmodel_yuv_t;

int cmodel_calculate_pixelsize(int colormodel);

void cmodel_transfer(unsigned char **output_rows, 
	unsigned char **input_rows,
	unsigned char *out_y_plane,
	unsigned char *out_u_plane,
	unsigned char *out_v_plane,
	unsigned char *in_y_plane,
	unsigned char *in_u_plane,
	unsigned char *in_v_plane,
	int in_x, 
	int in_y, 
	int in_w, 
	int in_h,
	int out_x, 
	int out_y, 
	int out_w, 
	int out_h,
	int in_colormodel, 
	int out_colormodel,
	int bg_color,
	int total_in_w,
	int total_out_w);

void cmodel_init_yuv(cmodel_yuv_t *yuv_table);
void cmodel_delete_yuv(cmodel_yuv_t *yuv_table);
int cmodel_bc_to_x(int color_model);
// Tell when to use plane arguments or row pointer arguments to functions
int cmodel_is_planar(int color_model);
void cmodel_to_text(char *string, int cmodel);
int cmodel_from_text(char *text);

#ifdef __cplusplus
}
#endif

#endif
