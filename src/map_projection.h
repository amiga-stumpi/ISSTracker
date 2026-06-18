#ifndef ISS_MAP_PROJECTION_H
#define ISS_MAP_PROJECTION_H
#include <exec/types.h>
void iss_project_equirect(WORD lat_cd, WORD lon_cd, WORD x, WORD y, WORD w, WORD h, WORD *px, WORD *py);
void iss_unproject_equirect(WORD px, WORD py, WORD x, WORD y, WORD w, WORD h, WORD *lat_cd, WORD *lon_cd);
#endif
