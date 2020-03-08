#ifndef FILE_INC
#define FILE_INC

#define PCM 1
#define WAV 2
#define MOV 3
#define PNG 4
#define JPEG 5
#define FILE_MPEG 8
#define FILE_TIFF 6      // because the TIFF library defines TIFF
#define GIF 7
#define JPEG_LIST 9

// For formats supported by plugins, the format number is the number in the 
// plugin list ORed with 0x8000.

#define MOV_NAME "Quicktime"
#define JPEG_LIST_NAME "JPEG List"

#define BITSLINEAR8 8
#define BITSLINEAR16 16
#define BITSLINEAR24 24
#define BITSWMX2 253
#define BITSULAW 254
#define BITSIMA4 255

// Video formats not part of Quicktime
#define MOV_RGBA "RGB ALPHA"
#define MOV_PNGA "PNG ALPHA"

class File;

#endif
