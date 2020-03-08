#ifndef VIDEODEVICE_INC
#define VIDEODEVICE_INC

class VideoDevice;

// Flags for mainwindow->video_driver
#define VIDEO4LINUX 0
#define VIDEO4LINUX2 1
#define SCREENCAPTURE 2
#define CAPTURE_LML 3
#define CAPTURE_FIREWIRE 4

#define VIDEO4LINUX_TITLE "Video4Linux"
#define VIDEO4LINUX2_TITLE "Video4Linux2"
#define SCREENCAPTURE_TITLE "Screencapture"
#define CAPTURE_LML_TITLE "LML 33"
#define CAPTURE_FIREWIRE_TITLE "IEEE 1394"

#define PLAYBACK_X11 0
#define PLAYBACK_LML 1
#define PLAYBACK_FIREWIRE 2

#define PLAYBACK_X11_TITLE "X11"
#define PLAYBACK_LML_TITLE "LML 33"
#define PLAYBACK_FIREWIRE_TITLE "IEEE 1394"

// Output formats required for direct frame copy
// RGB is for either RGB or RGBA as defined in the VFrame
#define OUTPUT_RGB 0
#define OUTPUT_MJPA 1
#define OUTPUT_DV 2

#endif
