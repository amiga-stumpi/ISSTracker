#ifndef ISS_MAP_IMAGE_H
#define ISS_MAP_IMAGE_H
#include <exec/types.h>
#define ISS_MAP_SRC_W 600
#define ISS_MAP_SRC_H 200
extern const UBYTE iss_map_2bpp[];
extern const UBYTE iss_map_4bpp[];
UBYTE iss_map_get_pen(UBYTE depth, WORD x, WORD y);
#endif
