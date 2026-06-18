#include <proto/graphics.h>
#include <string.h>
#include "draw.h"
#include "map_projection.h"
#include "worldmask.h"
#define MAP_X 8
#define MAP_Y 18
#define MAP_W 304
#define MAP_H 122
#define BTN_Y 148
#define BTN_W 48
#define BTN_H 14
static void text_at(struct RastPort *rp, WORD x, WORD y, const char *s){ Move(rp,x,y); Text(rp,(STRPTR)s,strlen(s)); }
static const char *status_name(UBYTE s){ if(s==ISS_STATUS_ONLINE) return "ONLINE"; if(s==ISS_STATUS_LOADING) return "LOADING"; if(s==ISS_STATUS_ERROR) return "ERROR"; return "OFFLINE"; }
static const char *light_name(UBYTE s){ if(s==ISS_LIGHT_DAY) return "DAY"; if(s==ISS_LIGHT_TWILIGHT) return "TWILIGHT"; return "NIGHT"; }
static const char *surf_name(UBYTE s){ if(s==ISS_SURFACE_LAND) return "LAND"; if(s==ISS_SURFACE_COAST) return "COAST"; return "WATER"; }
static void draw_map(struct RastPort *rp,UBYTE depth){ WORD x; WORD y; WORD lat; WORD lon; UBYTE c; SetAPen(rp,0); RectFill(rp,MAP_X,MAP_Y,MAP_X+MAP_W,MAP_Y+MAP_H); for(y=0;y<MAP_H;y+=3){ for(x=0;x<MAP_W;x+=4){ iss_unproject_equirect((WORD)(MAP_X+x),(WORD)(MAP_Y+y),MAP_X,MAP_Y,MAP_W,MAP_H,&lat,&lon); c=worldmask_classify(lat,lon); if(c==ISS_SURFACE_LAND) SetAPen(rp,(depth>=4)?3:1); else if(c==ISS_SURFACE_COAST) SetAPen(rp,(depth>=4)?2:1); else SetAPen(rp,0); RectFill(rp,MAP_X+x,MAP_Y+y,MAP_X+x+3,MAP_Y+y+2); } } SetAPen(rp,1); Move(rp,MAP_X,MAP_Y); Draw(rp,MAP_X+MAP_W,MAP_Y); Draw(rp,MAP_X+MAP_W,MAP_Y+MAP_H); Draw(rp,MAP_X,MAP_Y+MAP_H); Draw(rp,MAP_X,MAP_Y); }
static void draw_iss(struct RastPort *rp,IssTrackerApp *app){ WORD px; WORD py; UWORD i; UWORD idx; if(app->show_trail){ SetAPen(rp,2); for(i=0;i<app->trail_count;i++){ idx=(app->trail_head+ISS_TRAIL_MAX-app->trail_count+i)%ISS_TRAIL_MAX; if(app->trail[idx].valid){ iss_project_equirect(app->trail[idx].lat_cd,app->trail[idx].lon_cd,MAP_X,MAP_Y,MAP_W,MAP_H,&px,&py); RectFill(rp,px-1,py-1,px+1,py+1); } } } if(app->current.valid && app->blink){ iss_project_equirect(app->current.lat_cd,app->current.lon_cd,MAP_X,MAP_Y,MAP_W,MAP_H,&px,&py); SetAPen(rp,1); Move(rp,px-5,py); Draw(rp,px+5,py); Move(rp,px,py-5); Draw(rp,px,py+5); SetAPen(rp,3); RectFill(rp,px-2,py-2,px+2,py+2); } }
void draw_all(struct Window *win, IssTrackerApp *app){ struct RastPort *rp; UBYTE depth; rp=win->RPort; depth=(UBYTE)win->WScreen->BitMap.Depth; SetDrMd(rp,JAM1); SetAPen(rp,0); RectFill(rp,0,0,win->Width-1,win->Height-1); SetAPen(rp,1); text_at(rp,8,10,ISS_TITLE); draw_map(rp,depth); draw_iss(rp,app); SetAPen(rp,1); text_at(rp,8,178,status_name(app->status)); if(app->current.valid){ text_at(rp,72,178,light_name(app->light_state)); text_at(rp,152,178,surf_name(app->surface_state)); } text_at(rp,8,192,app->info_text); }
