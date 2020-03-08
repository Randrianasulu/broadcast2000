#ifndef CLIP_H
#define CLIP_H

#define CLIP(x, y, z) ((x) < (y) ? (y) : ((x) > (z) ? (z) : (x)))
#define RECLIP(x, y, z) ((x) = ((x) < (y) ? (y) : ((x) > (z) ? (z) : (x))))
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define MIN(x, y) ((x) < (y) ? (x) : (y))

#endif
