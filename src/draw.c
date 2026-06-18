#include <proto/graphics.h>
#include <string.h>
#include "draw.h"
#include "map_projection.h"
#include "worldmask.h"
#define PAD_X 8
#define TITLE_Y 10
#define MAP_TOP 18
#define BTN_W 58
#define BTN_H 16
#define STATUS_GAP 14
#define MIN_MAP_W 240
#define MIN_MAP_H 80
static void text_at(struct RastPort *rp, WORD x, WORD y, const char *s){ Move(rp,x,y); Text(rp,(STRPTR)s,strlen(s)); }
static const char *status_name(UBYTE s){ if(s==ISS_STATUS_ONLINE) return "ONLINE"; if(s==ISS_STATUS_LOADING) return "LOADING"; if(s==ISS_STATUS_ERROR) return "ERROR"; return "OFFLINE"; }
static const char *light_name(UBYTE s){ if(s==ISS_LIGHT_DAY) return "DAY"; if(s==ISS_LIGHT_TWILIGHT) return "TWILIGHT"; return "NIGHT"; }
static const char *surf_name(UBYTE s){ if(s==ISS_SURFACE_LAND) return "LAND"; if(s==ISS_SURFACE_COAST) return "COAST"; return "WATER"; }
static void layout(struct Window *win, WORD *mx, WORD *my, WORD *mw, WORD *mh, WORD *by, WORD *sy, WORD *iy){ WORD w; WORD h; w=win->Width; h=win->Height; *mx=PAD_X; *my=MAP_TOP; *mw=(WORD)(w-(PAD_X*2)-6); if(*mw<MIN_MAP_W) *mw=MIN_MAP_W; *mh=(WORD)(h-88); if(*mh<MIN_MAP_H) *mh=MIN_MAP_H; *by=(WORD)(*my+*mh+8); *sy=(WORD)(*by+BTN_H+STATUS_GAP); *iy=(WORD)(*sy+16); }
static void draw_button(struct RastPort *rp, WORD x, WORD y, const char *label, UBYTE active){ UBYTE fill; fill=active?2:1; SetAPen(rp,fill); RectFill(rp,x,y,x+BTN_W,y+BTN_H); SetAPen(rp,0); Move(rp,x+6,y+11); Text(rp,(STRPTR)label,strlen(label)); SetAPen(rp,3); Move(rp,x,y); Draw(rp,x+BTN_W,y); Draw(rp,x+BTN_W,y+BTN_H); Draw(rp,x,y+BTN_H); Draw(rp,x,y); }
static void draw_map(struct RastPort *rp, UBYTE depth, WORD map_x, WORD map_y, WORD map_w, WORD map_h){ WORD x; WORD y; WORD lat; WORD lon; UBYTE c; UBYTE ocean; UBYTE land; UBYTE coast; ocean=0; land=(depth>=4)?3:2; coast=(depth>=4)?2:1; SetAPen(rp,ocean); RectFill(rp,map_x,map_y,map_x+map_w,map_y+map_h); for(y=0;y<map_h;y+=2){ for(x=0;x<map_w;x+=3){ iss_unproject_equirect((WORD)(map_x+x),(WORD)(map_y+y),map_x,map_y,map_w,map_h,&lat,&lon); c=worldmask_classify(lat,lon); if(c==ISS_SURFACE_LAND) SetAPen(rp,land); else if(c==ISS_SURFACE_COAST) SetAPen(rp,coast); else SetAPen(rp,ocean); RectFill(rp,map_x+x,map_y+y,map_x+x+2,map_y+y+1); } } SetAPen(rp,1); Move(rp,map_x,map_y); Draw(rp,map_x+map_w,map_y); Draw(rp,map_x+map_w,map_y+map_h); Draw(rp,map_x,map_y+map_h); Draw(rp,map_x,map_y); Move(rp,map_x,map_y+map_h/2); Draw(rp,map_x+map_w,map_y+map_h/2); Move(rp,map_x+map_w/2,map_y); Draw(rp,map_x+map_w/2,map_y+map_h); }
static void draw_iss(struct RastPort *rp,IssTrackerApp *app, WORD map_x, WORD map_y, WORD map_w, WORD map_h){ WORD px; WORD py; UWORD i; UWORD idx; if(app->show_trail){ SetAPen(rp,2); for(i=0;i<app->trail_count;i++){ idx=(app->trail_head+ISS_TRAIL_MAX-app->trail_count+i)%ISS_TRAIL_MAX; if(app->trail[idx].valid){ iss_project_equirect(app->trail[idx].lat_cd,app->trail[idx].lon_cd,map_x,map_y,map_w,map_h,&px,&py); RectFill(rp,px-1,py-1,px+1,py+1); } } } if(app->current.valid && app->blink){ iss_project_equirect(app->current.lat_cd,app->current.lon_cd,map_x,map_y,map_w,map_h,&px,&py); SetAPen(rp,1); Move(rp,px-5,py); Draw(rp,px+5,py); Move(rp,px,py-5); Draw(rp,px,py+5); SetAPen(rp,3); RectFill(rp,px-2,py-2,px+2,py+2); } }
void draw_all(struct Window *win, IssTrackerApp *app){ struct RastPort *rp; UBYTE depth; WORD mx; WORD my; WORD mw; WORD mh; WORD by; WORD sy; WORD iy; rp=win->RPort; depth=(UBYTE)win->WScreen->BitMap.Depth; layout(win,&mx,&my,&mw,&mh,&by,&sy,&iy); SetDrMd(rp,JAM1); SetAPen(rp,0); RectFill(rp,0,0,win->Width-1,win->Height-1); SetAPen(rp,1); text_at(rp,PAD_X,TITLE_Y,ISS_TITLE); draw_map(rp,depth,mx,my,mw,mh); draw_iss(rp,app,mx,my,mw,mh); draw_button(rp,8,by,"Update",0); draw_button(rp,72,by,"Auto",app->auto_update); draw_button(rp,136,by,"Info",0); draw_button(rp,200,by,"Trail",app->show_trail); draw_button(rp,264,by,"Quit",0); SetAPen(rp,1); text_at(rp,8,sy,status_name(app->status)); if(app->current.valid){ text_at(rp,72,sy,light_name(app->light_state)); text_at(rp,152,sy,surf_name(app->surface_state)); } text_at(rp,8,iy,app->info_text); }
