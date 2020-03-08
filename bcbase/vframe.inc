#ifndef VFRAME_INC
#define VFRAME_INC


// size of a channel
//#define VWORD unsigned short int
#define VWORD unsigned char

// maximum value of a channel
//#define VMAX 65535
#define VMAX 255


// The maximum value is also used to determine the bit shifting to convert from
// unsigned char to VWORD.
// Get compiler warnings if bitshift is defined because >> can't take 0
// as an argument.
// 65535 forces bit shift of 8
// 255 disables bit shift

// Color models

#define VFRAME_YUV420  3
#define VFRAME_COMPRESSED 6  // Compressed data only

#define VFRAME_RGB565  6
#define VFRAME_RGBA8880 2
#define VFRAME_RGB888  0
#define VFRAME_BGRA8880 4
#define VFRAME_BGR888  5
#define VFRAME_VPIXEL 1      // RGB VPixel
#define VFRAME_VPIXELYUV 7   // YUV VPixel

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
