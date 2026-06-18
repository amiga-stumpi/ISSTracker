#include "map_projection.h"

void iss_project_equirect(WORD lat_cd, WORD lon_cd, WORD x, WORD y, WORD w, WORD h, WORD *px, WORD *py)
{
    LONG xx;
    LONG yy;

    if(w<1) w=1;
    if(h<1) h=1;

    xx=((LONG)(lon_cd+18000)*(LONG)w+18000L)/36000L;
    yy=((LONG)(9000-lat_cd)*(LONG)h+9000L)/18000L;
    if(xx<0) xx=0;
    if(yy<0) yy=0;
    if(xx>=w) xx=w-1;
    if(yy>=h) yy=h-1;

    *px=(WORD)(x+xx);
    *py=(WORD)(y+yy);
}

void iss_unproject_equirect(WORD px, WORD py, WORD x, WORD y, WORD w, WORD h, WORD *lat_cd, WORD *lon_cd)
{
    LONG lx;
    LONG ly;

    if(w<1) w=1;
    if(h<1) h=1;

    lx=px-x;
    ly=py-y;
    if(lx<0) lx=0;
    if(ly<0) ly=0;
    if(lx>=w) lx=w-1;
    if(ly>=h) ly=h-1;

    *lon_cd=(WORD)(((lx*36000L)+((LONG)w/2))/(LONG)w-18000L);
    *lat_cd=(WORD)(9000L-(((ly*18000L)+((LONG)h/2))/(LONG)h));
}
