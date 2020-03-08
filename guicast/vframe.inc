#ifndef VFRAME_INC
#define VFRAME_INC


// Size of a channel in the VPixel color model

//#define VWORD unsigned short int
#define VWORD unsigned char

// maximum value of a channel in the VPixel color model
//#define VMAX 65535
#define VMAX 255


// The maximum value is also used to determine the bit shifting to convert from
// unsigned char to VWORD.
// Get compiler warnings if bitshift is defined because >> can't take 0
// as an argument.
// 65535 forces bit shift of 8
// 255 disables bit shift

// Color models used by VFrame

#define VFRAME_YUV420  7
#define VFRAME_COMPRESSED 1  // Compressed data only

#define VFRAME_RGB565  3
#define VFRAME_RGB8880 10
#define VFRAME_RGB888  9
#define VFRAME_BGR8880 6
#define VFRAME_BGR888  5
#define VFRAME_RGBA8888 10    // Format for widgets
#define VFRAME_VPIXEL 10      // RGB VPixel
#define VFRAME_VPIXELYUV 14   // YUV VPixel

class VFrame;

typedef struct 
{
	VWORD r;
	VWORD g;
	VWORD b;
	VWORD a;
} VPixel;

typedef struct
{
	VWORD y;
	VWORD u;
	VWORD v;
	VWORD a;
} VPixelYUV;

#endif
