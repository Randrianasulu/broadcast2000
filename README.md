# broadcast2000
My hacks on Broadcast2000

Should work on gcc 5.5.0/x86-32 bit

Apparently, only NTSC DV works in this release.
Prepare such video with ffmpeg:
ffmpeg -i /dev/shm/capture.mov001.mov -c:v dvvideo -c:a pcm_s16be -s 720x480 -pix_fmt yuv411p /dev/shm/DV.mov
 
 or you can use mjpeg:
 ffmpeg -i 12.ogg -c:v mjpeg -b:v 9M -c:a pcm_s16be -s 1280x720 12.mov
 
for some reason on another machine I must use "-pix_fmt yuv420p" for mjpeg encoding, 
otherwise Broadcast2000 crashes in libjpeg-turbo

For compiling on x86_64 add "--no-mmx" arg to configure invokation

Requires libxv-dev and glib1.2-dev (glib 1.2.10 need some patching)
Nasm required on x86 for libjpeg-turbo
Autotools required for libjpeg-turbo

You can even capture modern webcam by using v4l1compat:
LD_PRELOAD=/usr/lib/libv4l/v4l1compat.so /usr/local/bcast/bcast2000.sh

