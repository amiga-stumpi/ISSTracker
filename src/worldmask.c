#include "app.h"
#include "worldmask.h"
typedef struct Box { WORD lat1,lat2,lon1,lon2; } Box;
static const Box land_boxes[] = { {700,7200,-16800,-5200},{800,5600,-12500,-6000},{-5600,1300,-8200,-3500},{3500,7200,-1000,6500},{-3500,3600,-1800,5200},{500,5600,2600,18000},{-4700,-1000,11000,15500},{-9000,-6500,-18000,18000},{-500,2000,9500,12500},{-4700,-3500,16500,18000},{5900,8300,-7400,-1200} };
UBYTE worldmask_classify(WORD lat_cd, WORD lon_cd){ UWORD i; for(i=0;i<sizeof(land_boxes)/sizeof(land_boxes[0]);i++){ const Box *b; b=&land_boxes[i]; if(lat_cd>=b->lat1 && lat_cd<=b->lat2 && lon_cd>=b->lon1 && lon_cd<=b->lon2){ if(lat_cd<b->lat1+300 || lat_cd>b->lat2-300 || lon_cd<b->lon1+300 || lon_cd>b->lon2-300) return ISS_SURFACE_COAST; return ISS_SURFACE_LAND; } } return ISS_SURFACE_WATER; }
