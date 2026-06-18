#ifndef ISS_CITYDB_H
#define ISS_CITYDB_H
#include <exec/types.h>
typedef struct City { const char *name; const char *country; WORD lat_cd; WORD lon_cd; UWORD rank; } City;
const City *citydb_nearest(WORD lat_cd, WORD lon_cd, UWORD *km_out);
UWORD citydb_count(void);
#endif
