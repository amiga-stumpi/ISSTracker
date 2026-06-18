#include <proto/graphics.h>
#include <string.h>
#include "draw.h"
#include "map_projection.h"
#include "worldmask.h"
#include "map_image.h"
#define PAD_X 8
#define TITLE_Y 10
#define MAP_TOP 18
#define BTN_W 58
#define BTN_H 16
#define STATUS_GAP 14
#define MIN_MAP_W 300
#define MIN_MAP_H 100
#define MARK_R 7
#define INFO_W 116
static void text_at(struct RastPort *rp, WORD x, WORD y, const char *s){ Move(rp,x,y); Text(rp,(STRPTR)s,strlen(s)); }
static const char *status_name(UBYTE s){ if(s==ISS_STATUS_ONLINE) return "Online"; if(s==ISS_STATUS_LOADING) return "Loading"; if(s==ISS_STATUS_ERROR) return "Error"; return "Offline"; }
static const char *light_name(UBYTE s){ if(s==ISS_LIGHT_DAY) return "DAY"; if(s==ISS_LIGHT_TWILIGHT) return "TWILIGHT"; return "NIGHT"; }
static const char *surf_name(UBYTE s){ if(s==ISS_SURFACE_LAND) return "LAND"; if(s==ISS_SURFACE_COAST) return "COAST"; return "WATER"; }
static void layout(struct Window *win, WORD *mx, WORD *my, WORD *mw, WORD *mh, WORD *by, WORD *sy, WORD *iy){ WORD w; WORD h; WORD reserve; w=win->Width; h=win->Height; reserve=(w>=450)?INFO_W:0; *mx=PAD_X; *my=MAP_TOP; *mw=(WORD)(w-(PAD_X*2)-6-reserve); if(*mw<MIN_MAP_W) *mw=MIN_MAP_W; *mh=(WORD)((LONG)(*mw)*ISS_MAP_SRC_H/ISS_MAP_SRC_W); if(*mh>(WORD)(h-88)) *mh=(WORD)(h-88); if(*mh<MIN_MAP_H) *mh=MIN_MAP_H; *by=(WORD)(*my+*mh+8); *sy=(WORD)(*by+BTN_H+STATUS_GAP); *iy=(WORD)(*sy+16); }
static void draw_button(struct RastPort *rp, WORD x, WORD y, const char *label, UBYTE active){ if(active){ SetAPen(rp,0); RectFill(rp,x,y,x+BTN_W,y+BTN_H); SetAPen(rp,1); Move(rp,x+7,y+12); Text(rp,(STRPTR)label,strlen(label)); SetAPen(rp,1); Move(rp,x+2,y+2); Draw(rp,x+BTN_W-2,y+2); Draw(rp,x+BTN_W-2,y+BTN_H-2); Draw(rp,x+2,y+BTN_H-2); Draw(rp,x+2,y+2); SetAPen(rp,3); Move(rp,x,y); Draw(rp,x+BTN_W,y); Draw(rp,x+BTN_W,y+BTN_H); Draw(rp,x,y+BTN_H); Draw(rp,x,y); } else { SetAPen(rp,1); RectFill(rp,x,y,x+BTN_W,y+BTN_H); SetAPen(rp,0); Move(rp,x+6,y+11); Text(rp,(STRPTR)label,strlen(label)); SetAPen(rp,3); Move(rp,x,y); Draw(rp,x+BTN_W,y); Draw(rp,x+BTN_W,y+BTN_H); Draw(rp,x,y+BTN_H); Draw(rp,x,y); } }
static void append_num(char *b, UWORD *p, LONG n){ char t[12]; WORD i; WORD j; if(n<0){ b[(*p)++]='-'; n=-n; } i=0; do{ t[i++]=(char)('0'+(n%10)); n/=10; }while(n&&i<11); for(j=i-1;j>=0;j--) b[(*p)++]=t[j]; b[*p]=0; }
static void label_num(struct RastPort *rp, WORD x, WORD y, const char *label, LONG value, const char *unit){ char buf[32]; UWORD p; const char *s; p=0; s=label; while(*s && p<sizeof(buf)-1) buf[p++]=*s++; append_num(buf,&p,value); s=unit; while(*s && p<sizeof(buf)-1) buf[p++]=*s++; buf[p]=0; text_at(rp,x,y,buf); }
static ULONG orbit_number(ULONG ts){ return (ULONG)(ts/5576UL); }
static UBYTE near_europe(IssTrackerApp *app){ if(!app->current.valid) return 0; if(app->current.lat_cd>=3500 && app->current.lat_cd<=7200 && app->current.lon_cd>=-2500 && app->current.lon_cd<=4500) return 2; if(app->current.lat_cd>=2500 && app->current.lat_cd<=7500 && app->current.lon_cd>=-6000 && app->current.lon_cd<=5500) return 1; return 0; }
static void draw_side_info(struct RastPort *rp, IssTrackerApp *app, WORD map_x, WORD map_y, WORD map_w, WORD map_h, WORD win_w)
{
    WORD x;
    WORD y;
    UBYTE europe;
    x=(WORD)(map_x+map_w+8);
    if((WORD)(x+INFO_W-12)>=win_w) return;
    y=map_y;
    SetAPen(rp,0);
    RectFill(rp,x,y,x+INFO_W-12,(WORD)(y+map_h));
    SetAPen(rp,1);
    Move(rp,x,y); Draw(rp,x+INFO_W-12,y); Draw(rp,x+INFO_W-12,y+map_h); Draw(rp,x,y+map_h); Draw(rp,x,y);
    text_at(rp,(WORD)(x+4),(WORD)(y+14),"Zoom Info");
    if(!app->current.valid){ text_at(rp,(WORD)(x+4),(WORD)(y+34),"Keine Pos"); return; }
    europe=near_europe(app);
    if(europe==2) text_at(rp,(WORD)(x+4),(WORD)(y+32),"Ueber Europa");
    else if(europe==1) text_at(rp,(WORD)(x+4),(WORD)(y+32),"Nahe Europa");
    else text_at(rp,(WORD)(x+4),(WORD)(y+32),"Global");
    label_num(rp,(WORD)(x+4),(WORD)(y+52),"Hoehe ",420," km");
    label_num(rp,(WORD)(x+4),(WORD)(y+68),"Tempo ",27600," km/h");
    label_num(rp,(WORD)(x+4),(WORD)(y+84),"Orbit ",(LONG)orbit_number(app->current.timestamp),"");
    label_num(rp,(WORD)(x+4),(WORD)(y+100),"Aktual ",(LONG)app->current.timestamp,"");
}
static void draw_map_rect(struct RastPort *rp, UBYTE depth, WORD map_x, WORD map_y, WORD map_w, WORD map_h, WORD rx1, WORD ry1, WORD rx2, WORD ry2){ WORD y; WORD x; WORD sx; WORD sy; WORD run_start; UBYTE pen; UBYTE last; if(rx1<0) rx1=0; if(ry1<0) ry1=0; if(rx2>=map_w) rx2=(WORD)(map_w-1); if(ry2>=map_h) ry2=(WORD)(map_h-1); for(y=ry1;y<=ry2;y++){ sy=(WORD)(((LONG)y*ISS_MAP_SRC_H)/map_h); run_start=rx1; sx=(WORD)(((LONG)rx1*ISS_MAP_SRC_W)/map_w); last=iss_map_get_pen(depth,sx,sy); for(x=(WORD)(rx1+1);x<=rx2;x++){ sx=(WORD)(((LONG)x*ISS_MAP_SRC_W)/map_w); pen=iss_map_get_pen(depth,sx,sy); if(pen!=last){ SetAPen(rp,last); Move(rp,map_x+run_start,map_y+y); Draw(rp,map_x+x-1,map_y+y); run_start=x; last=pen; } } SetAPen(rp,last); Move(rp,map_x+run_start,map_y+y); Draw(rp,map_x+rx2,map_y+y); } }
static void draw_map(struct RastPort *rp, UBYTE depth, WORD map_x, WORD map_y, WORD map_w, WORD map_h){ draw_map_rect(rp,depth,map_x,map_y,map_w,map_h,0,0,(WORD)(map_w-1),(WORD)(map_h-1)); SetAPen(rp,1); Move(rp,map_x,map_y); Draw(rp,map_x+map_w,map_y); Draw(rp,map_x+map_w,map_y+map_h); Draw(rp,map_x,map_y+map_h); Draw(rp,map_x,map_y); }
static void draw_marker_shape(struct RastPort *rp, WORD px, WORD py){ SetAPen(rp,1); Move(rp,px-5,py); Draw(rp,px+5,py); Move(rp,px,py-5); Draw(rp,px,py+5); SetAPen(rp,3); RectFill(rp,px-2,py-2,px+2,py+2); }
static void draw_trail_clip(struct RastPort *rp, IssTrackerApp *app, WORD map_x, WORD map_y, WORD map_w, WORD map_h, WORD rx1, WORD ry1, WORD rx2, WORD ry2){ WORD px; WORD py; WORD lx; WORD ly; UWORD i; UWORD idx; if(!app->show_trail) return; SetAPen(rp,2); for(i=0;i<app->trail_count;i++){ idx=(app->trail_head+ISS_TRAIL_MAX-app->trail_count+i)%ISS_TRAIL_MAX; if(app->trail[idx].valid){ iss_project_equirect(app->trail[idx].lat_cd,app->trail[idx].lon_cd,map_x,map_y,map_w,map_h,&px,&py); lx=(WORD)(px-map_x); ly=(WORD)(py-map_y); if(lx>=rx1-1 && lx<=rx2+1 && ly>=ry1-1 && ly<=ry2+1) RectFill(rp,px-1,py-1,px+1,py+1); } } }
static void draw_iss(struct RastPort *rp,IssTrackerApp *app, WORD map_x, WORD map_y, WORD map_w, WORD map_h){ WORD px; WORD py; draw_trail_clip(rp,app,map_x,map_y,map_w,map_h,0,0,(WORD)(map_w-1),(WORD)(map_h-1)); if(app->current.valid && app->blink){ iss_project_equirect(app->current.lat_cd,app->current.lon_cd,map_x,map_y,map_w,map_h,&px,&py); draw_marker_shape(rp,px,py); } }
void draw_iss_blink(struct Window *win, IssTrackerApp *app){ struct RastPort *rp; UBYTE depth; WORD mx; WORD my; WORD mw; WORD mh; WORD by; WORD sy; WORD iy; WORD px; WORD py; WORD lx; WORD ly; WORD rx1; WORD ry1; WORD rx2; WORD ry2; if(!app->current.valid) return; rp=win->RPort; depth=(UBYTE)win->WScreen->BitMap.Depth; layout(win,&mx,&my,&mw,&mh,&by,&sy,&iy); iss_project_equirect(app->current.lat_cd,app->current.lon_cd,mx,my,mw,mh,&px,&py); lx=(WORD)(px-mx); ly=(WORD)(py-my); rx1=(WORD)(lx-MARK_R); ry1=(WORD)(ly-MARK_R); rx2=(WORD)(lx+MARK_R); ry2=(WORD)(ly+MARK_R); SetDrMd(rp,JAM1); draw_map_rect(rp,depth,mx,my,mw,mh,rx1,ry1,rx2,ry2); draw_trail_clip(rp,app,mx,my,mw,mh,rx1,ry1,rx2,ry2); if(app->blink) draw_marker_shape(rp,px,py); }
void draw_side_panel(struct Window *win, IssTrackerApp *app){ struct RastPort *rp; WORD mx; WORD my; WORD mw; WORD mh; WORD by; WORD sy; WORD iy; rp=win->RPort; layout(win,&mx,&my,&mw,&mh,&by,&sy,&iy); SetDrMd(rp,JAM1); draw_side_info(rp,app,mx,my,mw,mh,win->Width); }
void draw_panel(struct Window *win, IssTrackerApp *app){ struct RastPort *rp; WORD mx; WORD my; WORD mw; WORD mh; WORD by; WORD sy; WORD iy; rp=win->RPort; layout(win,&mx,&my,&mw,&mh,&by,&sy,&iy); SetDrMd(rp,JAM1); SetAPen(rp,0); RectFill(rp,0,by-2,win->Width-1,win->Height-1); draw_button(rp,8,by,"Update",0); draw_button(rp,72,by,"Info",0); draw_button(rp,136,by,"Quit",0); SetAPen(rp,1); text_at(rp,8,sy,"Status:"); text_at(rp,62,sy,status_name(app->status)); if(app->current.valid){ text_at(rp,116,sy,"*  DAYTIME:"); text_at(rp,210,sy,light_name(app->light_state)); text_at(rp,258,sy,"*  Terrain:"); text_at(rp,350,sy,surf_name(app->surface_state)); } text_at(rp,8,iy,app->info_text); }
void draw_all(struct Window *win, IssTrackerApp *app){ struct RastPort *rp; UBYTE depth; WORD mx; WORD my; WORD mw; WORD mh; WORD by; WORD sy; WORD iy; rp=win->RPort; depth=(UBYTE)win->WScreen->BitMap.Depth; layout(win,&mx,&my,&mw,&mh,&by,&sy,&iy); SetDrMd(rp,JAM1); SetAPen(rp,0); RectFill(rp,0,0,win->Width-1,win->Height-1); SetAPen(rp,1); text_at(rp,PAD_X,TITLE_Y,ISS_TITLE); draw_map(rp,depth,mx,my,mw,mh); draw_iss(rp,app,mx,my,mw,mh); draw_side_info(rp,app,mx,my,mw,mh,win->Width); draw_panel(win,app); }
