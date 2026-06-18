#include <proto/graphics.h>
#include <string.h>
#include "draw.h"
#include "map_projection.h"
#include "worldmask.h"
#define MAP_X 8
#define MAP_Y 18
#define MAP_W 360
#define MAP_H 132
#define BTN_Y 160
#define BTN_W 58
#define BTN_H 16
static void text_at(struct RastPort *rp, WORD x, WORD y, const char *s){ Move(rp,x,y); Text(rp,(STRPTR)s,strlen(s)); }
static const char *status_name(UBYTE s){ if(s==ISS_STATUS_ONLINE) return "ONLINE"; if(s==ISS_STATUS_LOADING) return "LOADING"; if(s==ISS_STATUS_ERROR) return "ERROR"; return "OFFLINE"; }
static const char *light_name(UBYTE s){ if(s==ISS_LIGHT_DAY) return "DAY"; if(s==ISS_LIGHT_TWILIGHT) return "TWILIGHT"; return "NIGHT"; }
static const char *surf_name(UBYTE s){ if(s==ISS_SURFACE_LAND) return "LAND"; if(s==ISS_SURFACE_COAST) return "COAST"; return "WATER"; }
static void draw_button(struct RastPort *rp, WORD x, const char *label, UBYTE active){ UBYTE fill; fill=active?2:1; SetAPen(rp,fill); RectFill(rp,x,BTN_Y,x+BTN_W,BTN_Y+BTN_H); SetAPen(rp,0); Move(rp,x+6,BTN_Y+11); Text(rp,(STRPTR)label,strlen(label)); SetAPen(rp,3); Move(rp,x,BTN_Y); Draw(rp,x+BTN_W,BTN_Y); Draw(rp,x+BTN_W,BTN_Y+BTN_H); Draw(rp,x,BTN_Y+BTN_H); Draw(rp,x,BTN_Y); }
static void draw_map(struct RastPort *rp,UBYTE depth){ WORD x; WORD y; WORD lat; WORD lon; UBYTE c; UBYTE ocean; UBYTE land; UBYTE coast; ocean=0; land=(depth>=4)?3:2; coast=(depth>=4)?2:1; SetAPen(rp,ocean); RectFill(rp,MAP_X,MAP_Y,MAP_X+MAP_W,MAP_Y+MAP_H); for(y=0;y<MAP_H;y+=2){ for(x=0;x<MAP_W;x+=3){ iss_unproject_equirect((WORD)(MAP_X+x),(WORD)(MAP_Y+y),MAP_X,MAP_Y,MAP_W,MAP_H,&lat,&lon); c=worldmask_classify(lat,lon); if(c==ISS_SURFACE_LAND) SetAPen(rp,land); else if(c==ISS_SURFACE_COAST) SetAPen(rp,coast); else SetAPen(rp,ocean); RectFill(rp,MAP_X+x,MAP_Y+y,MAP_X+x+2,MAP_Y+y+1); } } SetAPen(rp,1); Move(rp,MAP_X,MAP_Y); Draw(rp,MAP_X+MAP_W,MAP_Y); Draw(rp,MAP_X+MAP_W,MAP_Y+MAP_H); Draw(rp,MAP_X,MAP_Y+MAP_H); Draw(rp,MAP_X,MAP_Y); Move(rp,MAP_X,MAP_Y+MAP_H/2); Draw(rp,MAP_X+MAP_W,MAP_Y+MAP_H/2); Move(rp,MAP_X+MAP_W/2,MAP_Y); Draw(rp,MAP_X+MAP_W/2,MAP_Y+MAP_H); }
static void draw_iss(struct RastPort *rp,IssTrackerApp *app){ WORD px; WORD py; UWORD i; UWORD idx; if(app->show_trail){ SetAPen(rp,2); for(i=0;i<app->trail_count;i++){ idx=(app->trail_head+ISS_TRAIL_MAX-app->trail_count+i)%ISS_TRAIL_MAX; if(app->trail[idx].valid){ iss_project_equirect(app->trail[idx].lat_cd,app->trail[idx].lon_cd,MAP_X,MAP_Y,MAP_W,MAP_H,&px,&py); RectFill(rp,px-1,py-1,px+1,py+1); } } } if(app->current.valid && app->blink){ iss_project_equirect(app->current.lat_cd,app->current.lon_cd,MAP_X,MAP_Y,MAP_W,MAP_H,&px,&py); SetAPen(rp,1); Move(rp,px-5,py); Draw(rp,px+5,py); Move(rp,px,py-5); Draw(rp,px,py+5); SetAPen(rp,3); RectFill(rp,px-2,py-2,px+2,py+2); } }
void draw_all(struct Window *win, IssTrackerApp *app){ struct RastPort *rp; UBYTE depth; rp=win->RPort; depth=(UBYTE)win->WScreen->BitMap.Depth; SetDrMd(rp,JAM1); SetAPen(rp,0); RectFill(rp,0,0,win->Width-1,win->Height-1); SetAPen(rp,1); text_at(rp,8,10,ISS_TITLE); draw_map(rp,depth); draw_iss(rp,app); draw_button(rp,8,"Update",0); draw_button(rp,72,"Auto",app->auto_update); draw_button(rp,136,"Info",0); draw_button(rp,200,"Trail",app->show_trail); draw_button(rp,264,"Quit",0); SetAPen(rp,1); text_at(rp,8,190,status_name(app->status)); if(app->current.valid){ text_at(rp,72,190,light_name(app->light_state)); text_at(rp,152,190,surf_name(app->surface_state)); } text_at(rp,8,206,app->info_text); }
