# broadcast2000
My hacks on Broadcast2000

Should work on gcc 5.5.0/x86-32 bit

Apparently, only NTSC DV works in this release.
Prepare such video with ffmpeg:
ffmpeg -i /dev/shm/capture.mov001.mov -c:v dvvideo -c:a pcm_s16be -s 720x480 -pix_fmt yuv411p /dev/shm/DV.mov
 
 or you can use mjpeg:
 ffmpeg -i 12.ogg -c:v mjpeg -b:v 9M -c:a pcm_s16be -s 1280x720 12.mov
 
