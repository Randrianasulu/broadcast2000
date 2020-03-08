#ifndef AUDIODEVICE_INC
#define AUDIODEVICE_INC

class AudioDevice;

#define TOTAL_BUFFERS 2

// Supported devices
#define AUDIO_OSS 0
#define AUDIO_ESOUND 1
#define AUDIO_NAS 2
#define AUDIO_ALSA 3
#define AUDIO_1394 4

#define AUDIO_OSS_TITLE "OSS"
#define AUDIO_ESOUND_TITLE "ESound"
#define AUDIO_NAS_TITLE "NAS"
#define AUDIO_ALSA_TITLE "ALSA"
#define AUDIO_1394_TITLE "IEEE1394"

#endif
