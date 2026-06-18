#include <proto/graphics.h>
#include <string.h>
#include "draw.h"
#include "map_projection.h"
#include "worldmask.h"
#include "map_image.h"
#include "funfacts.h"
#define PAD_X 8
#define TITLE_Y 10
#define MAP_TOP 18
#define BTN_W 58
#define BTN_H 16
#define STATUS_GAP 13
#define MIN_MAP_W 300
#define MIN_MAP_H 100
#define MARK_R 7
static void text_at(struct RastPort *rp, WORD x, WORD y, const char *s){ Move(rp,x,y); Text(rp,(STRPTR)s,strlen(s)); }
static const char *status_name(IssTrackerApp *app){ if(app->language==ISS_LANG_DE){ if(app->status==ISS_STATUS_ONLINE) return "Online"; if(app->status==ISS_STATUS_LOADING) return "Laden"; if(app->status==ISS_STATUS_ERROR) return "Fehler"; return "Offline"; } if(app->language==ISS_LANG_PL){ if(app->status==ISS_STATUS_ONLINE) return "Online"; if(app->status==ISS_STATUS_LOADING) return "Ladowanie"; if(app->status==ISS_STATUS_ERROR) return "Blad"; return "Offline"; } if(app->status==ISS_STATUS_ONLINE) return "Online"; if(app->status==ISS_STATUS_LOADING) return "Loading"; if(app->status==ISS_STATUS_ERROR) return "Error"; return "Offline"; }
static const char *light_name(IssTrackerApp *app){ if(app->language==ISS_LANG_DE){ if(app->light_state==ISS_LIGHT_DAY) return "Tag"; if(app->light_state==ISS_LIGHT_TWILIGHT) return "Daemmerung"; return "Nacht"; } if(app->language==ISS_LANG_PL){ if(app->light_state==ISS_LIGHT_DAY) return "Dzien"; if(app->light_state==ISS_LIGHT_TWILIGHT) return "Zmierzch"; return "Noc"; } if(app->light_state==ISS_LIGHT_DAY) return "DAY"; if(app->light_state==ISS_LIGHT_TWILIGHT) return "TWILIGHT"; return "NIGHT"; }
static const char *surf_name(IssTrackerApp *app){ if(app->language==ISS_LANG_DE){ if(app->surface_state==ISS_SURFACE_LAND) return "Land"; if(app->surface_state==ISS_SURFACE_COAST) return "Kueste"; return "Wasser"; } if(app->language==ISS_LANG_PL){ if(app->surface_state==ISS_SURFACE_LAND) return "Lad"; if(app->surface_state==ISS_SURFACE_COAST) return "Wybrzeze"; return "Woda"; } if(app->surface_state==ISS_SURFACE_LAND) return "LAND"; if(app->surface_state==ISS_SURFACE_COAST) return "COAST"; return "WATER"; }
static const char *label_status(IssTrackerApp *app){ if(app->language==ISS_LANG_DE) return "Status: "; if(app->language==ISS_LANG_PL) return "Status: "; return "Status: "; }
static const char *label_daytime(IssTrackerApp *app){ if(app->language==ISS_LANG_DE) return "  *  Tageszeit: "; if(app->language==ISS_LANG_PL) return "  *  Pora dnia: "; return "  *  Daytime: "; }
static const char *label_terrain(IssTrackerApp *app){ if(app->language==ISS_LANG_DE) return "  *  Grund: "; if(app->language==ISS_LANG_PL) return "  *  Teren: "; return "  *  Terrain: "; }

static void layout(struct Window *win, WORD *mx, WORD *my, WORD *mw, WORD *mh, WORD *by, WORD *sy, WORD *iy){ WORD w; WORD h; w=win->Width; h=win->Height; *mx=PAD_X; *my=MAP_TOP; *mw=(WORD)(w-(PAD_X*2)-6); if(*mw<MIN_MAP_W) *mw=MIN_MAP_W; *mh=(WORD)((LONG)(*mw)*ISS_MAP_SRC_H/ISS_MAP_SRC_W); if(*mh>(WORD)(h-104)) *mh=(WORD)(h-104); if(*mh<MIN_MAP_H) *mh=MIN_MAP_H; *by=(WORD)(*my+*mh+8); *sy=(WORD)(*by+BTN_H+STATUS_GAP); *iy=(WORD)(*sy+16); }
static void draw_button(struct RastPort *rp, WORD x, WORD y, const char *label, UBYTE active){ if(active){ SetAPen(rp,0); RectFill(rp,x,y,x+BTN_W,y+BTN_H); SetAPen(rp,1); Move(rp,x+7,y+12); Text(rp,(STRPTR)label,strlen(label)); SetAPen(rp,1); Move(rp,x+2,y+2); Draw(rp,x+BTN_W-2,y+2); Draw(rp,x+BTN_W-2,y+BTN_H-2); Draw(rp,x+2,y+BTN_H-2); Draw(rp,x+2,y+2); SetAPen(rp,3); Move(rp,x,y); Draw(rp,x+BTN_W,y); Draw(rp,x+BTN_W,y+BTN_H); Draw(rp,x,y+BTN_H); Draw(rp,x,y); } else { SetAPen(rp,1); RectFill(rp,x,y,x+BTN_W,y+BTN_H); SetAPen(rp,0); Move(rp,x+6,y+11); Text(rp,(STRPTR)label,strlen(label)); SetAPen(rp,3); Move(rp,x,y); Draw(rp,x+BTN_W,y); Draw(rp,x+BTN_W,y+BTN_H); Draw(rp,x,y+BTN_H); Draw(rp,x,y); } }
static void append_num(char *b, UWORD *p, LONG n){ char t[12]; WORD i; WORD j; if(n<0){ b[(*p)++]='-'; n=-n; } i=0; do{ t[i++]=(char)('0'+(n%10)); n/=10; }while(n&&i<11); for(j=i-1;j>=0;j--) b[(*p)++]=t[j]; b[*p]=0; }
static void append_str(char *b, UWORD *p, const char *s){ while(*s && *p<118) b[(*p)++]=*s++; b[*p]=0; }
static ULONG orbit_number(ULONG ts){ return (ULONG)(ts/5576UL); }
static UBYTE near_europe(IssTrackerApp *app){ if(!app->current.valid) return 0; if(app->current.lat_cd>=3500 && app->current.lat_cd<=7200 && app->current.lon_cd>=-2500 && app->current.lon_cd<=4500) return 2; if(app->current.lat_cd>=2500 && app->current.lat_cd<=7500 && app->current.lon_cd>=-6000 && app->current.lon_cd<=5500) return 1; return 0; }
static void make_status_line(IssTrackerApp *app, char *buf)
{
    UWORD p;
    p=0;
    append_str(buf,&p,label_status(app));
    append_str(buf,&p,status_name(app));
    if(app->current.valid){
        append_str(buf,&p,label_daytime(app));
        append_str(buf,&p,light_name(app));
        append_str(buf,&p,label_terrain(app));
        append_str(buf,&p,surf_name(app));
    }
}
static void make_detail_line(IssTrackerApp *app, char *buf)
{
    UWORD p;
    UBYTE europe;
    p=0;
    if(app->funfact_active){ append_str(buf,&p,funfact_get(app->funfact_index,app->language)); return; }
    if(app->status_page==0){ append_str(buf,&p,app->info_text); return; }
    if(!app->current.valid){ append_str(buf,&p,app->info_text); return; }
    if(app->status_page==1){ if(app->language==ISS_LANG_PL) append_str(buf,&p,"Pozycja: Lat "); else append_str(buf,&p,"Position: Lat "); append_num(buf,&p,app->current.lat_cd/100); append_str(buf,&p,"  Lon "); append_num(buf,&p,app->current.lon_cd/100); return; }
    if(app->status_page==2){ if(app->language==ISS_LANG_EN) append_str(buf,&p,"ISS: Altitude 420 km  *  Speed 27600 km/h  *  Orbit "); else if(app->language==ISS_LANG_PL) append_str(buf,&p,"ISS: Wysokosc 420 km  *  Predkosc 27600 km/h  *  Orbita "); else append_str(buf,&p,"ISS: Hoehe 420 km  *  Tempo 27600 km/h  *  Orbit "); append_num(buf,&p,(LONG)orbit_number(app->current.timestamp)); return; }
    europe=near_europe(app);
    if(app->language==ISS_LANG_EN){ if(europe==2) append_str(buf,&p,"Notice: currently over Europe"); else if(europe==1) append_str(buf,&p,"Notice: ISS approaching Europe"); else append_str(buf,&p,"Notice: ISS outside Europe"); append_str(buf,&p,"  *  Updated "); } else if(app->language==ISS_LANG_PL){ if(europe==2) append_str(buf,&p,"Info: teraz nad Europa"); else if(europe==1) append_str(buf,&p,"Info: ISS zbliza sie do Europy"); else append_str(buf,&p,"Info: ISS poza Europa"); append_str(buf,&p,"  *  Aktual "); } else { if(europe==2) append_str(buf,&p,"Hinweis: derzeit ueber Europa"); else if(europe==1) append_str(buf,&p,"Hinweis: ISS naehert sich Europa"); else append_str(buf,&p,"Hinweis: ISS ausserhalb Europa"); append_str(buf,&p,"  *  Aktual "); }
    append_num(buf,&p,(LONG)app->current.timestamp);
}
static void draw_map_rect(struct RastPort *rp, UBYTE depth, WORD map_x, WORD map_y, WORD map_w, WORD map_h, WORD rx1, WORD ry1, WORD rx2, WORD ry2){ WORD y; WORD x; WORD sx; WORD sy; WORD run_start; UBYTE pen; UBYTE last; if(rx1<0) rx1=0; if(ry1<0) ry1=0; if(rx2>=map_w) rx2=(WORD)(map_w-1); if(ry2>=map_h) ry2=(WORD)(map_h-1); for(y=ry1;y<=ry2;y++){ sy=(WORD)(((LONG)y*ISS_MAP_SRC_H)/map_h); run_start=rx1; sx=(WORD)(((LONG)rx1*ISS_MAP_SRC_W)/map_w); last=iss_map_get_pen(depth,sx,sy); for(x=(WORD)(rx1+1);x<=rx2;x++){ sx=(WORD)(((LONG)x*ISS_MAP_SRC_W)/map_w); pen=iss_map_get_pen(depth,sx,sy); if(pen!=last){ SetAPen(rp,last); Move(rp,map_x+run_start,map_y+y); Draw(rp,map_x+x-1,map_y+y); run_start=x; last=pen; } } SetAPen(rp,last); Move(rp,map_x+run_start,map_y+y); Draw(rp,map_x+rx2,map_y+y); } }
static void draw_map(struct RastPort *rp, UBYTE depth, WORD map_x, WORD map_y, WORD map_w, WORD map_h){ draw_map_rect(rp,depth,map_x,map_y,map_w,map_h,0,0,(WORD)(map_w-1),(WORD)(map_h-1)); SetAPen(rp,1); Move(rp,map_x,map_y); Draw(rp,map_x+map_w,map_y); Draw(rp,map_x+map_w,map_y+map_h); Draw(rp,map_x,map_y+map_h); Draw(rp,map_x,map_y); }
static void draw_marker_shape(struct RastPort *rp, WORD px, WORD py){ SetAPen(rp,1); Move(rp,px-5,py); Draw(rp,px+5,py); Move(rp,px,py-5); Draw(rp,px,py+5); SetAPen(rp,3); RectFill(rp,px-2,py-2,px+2,py+2); }
static void draw_trail_clip(struct RastPort *rp, IssTrackerApp *app, UBYTE depth, WORD map_x, WORD map_y, WORD map_w, WORD map_h, WORD rx1, WORD ry1, WORD rx2, WORD ry2){ WORD px; WORD py; WORD lx; WORD ly; UWORD i; UWORD idx; if(depth<4 && !app->show_trail) return; SetAPen(rp,(depth>=4)?0:2); for(i=0;i<app->trail_count;i++){ idx=(app->trail_head+ISS_TRAIL_MAX-app->trail_count+i)%ISS_TRAIL_MAX; if(app->trail[idx].valid){ iss_project_equirect(app->trail[idx].lat_cd,app->trail[idx].lon_cd,map_x,map_y,map_w,map_h,&px,&py); lx=(WORD)(px-map_x); ly=(WORD)(py-map_y); if(lx>=rx1-1 && lx<=rx2+1 && ly>=ry1-1 && ly<=ry2+1) RectFill(rp,px-1,py-1,px+1,py+1); } } }
static void draw_iss(struct RastPort *rp,IssTrackerApp *app, UBYTE depth, WORD map_x, WORD map_y, WORD map_w, WORD map_h){ WORD px; WORD py; draw_trail_clip(rp,app,depth,map_x,map_y,map_w,map_h,0,0,(WORD)(map_w-1),(WORD)(map_h-1)); if(app->current.valid && app->blink){ iss_project_equirect(app->current.lat_cd,app->current.lon_cd,map_x,map_y,map_w,map_h,&px,&py); draw_marker_shape(rp,px,py); } }
void draw_iss_blink(struct Window *win, IssTrackerApp *app){ struct RastPort *rp; UBYTE depth; WORD mx; WORD my; WORD mw; WORD mh; WORD by; WORD sy; WORD iy; WORD px; WORD py; WORD lx; WORD ly; WORD rx1; WORD ry1; WORD rx2; WORD ry2; if(!app->current.valid) return; rp=win->RPort; depth=(UBYTE)win->WScreen->BitMap.Depth; layout(win,&mx,&my,&mw,&mh,&by,&sy,&iy); iss_project_equirect(app->current.lat_cd,app->current.lon_cd,mx,my,mw,mh,&px,&py); lx=(WORD)(px-mx); ly=(WORD)(py-my); rx1=(WORD)(lx-MARK_R); ry1=(WORD)(ly-MARK_R); rx2=(WORD)(lx+MARK_R); ry2=(WORD)(ly+MARK_R); SetDrMd(rp,JAM1); draw_map_rect(rp,depth,mx,my,mw,mh,rx1,ry1,rx2,ry2); draw_trail_clip(rp,app,depth,mx,my,mw,mh,rx1,ry1,rx2,ry2); if(app->blink) draw_marker_shape(rp,px,py); }
void draw_panel(struct Window *win, IssTrackerApp *app){ struct RastPort *rp; WORD mx; WORD my; WORD mw; WORD mh; WORD by; WORD sy; WORD iy; char line[120]; rp=win->RPort; layout(win,&mx,&my,&mw,&mh,&by,&sy,&iy); SetDrMd(rp,JAM1); SetAPen(rp,0); RectFill(rp,0,by-2,win->Width-1,win->Height-1); draw_button(rp,8,by,"Update",0); draw_button(rp,72,by,"Info",0); SetAPen(rp,1); make_status_line(app,line); text_at(rp,8,sy,line); make_detail_line(app,line); text_at(rp,8,iy,line); }
void draw_all(struct Window *win, IssTrackerApp *app){ struct RastPort *rp; UBYTE depth; WORD mx; WORD my; WORD mw; WORD mh; WORD by; WORD sy; WORD iy; rp=win->RPort; depth=(UBYTE)win->WScreen->BitMap.Depth; layout(win,&mx,&my,&mw,&mh,&by,&sy,&iy); SetDrMd(rp,JAM1); SetAPen(rp,0); RectFill(rp,0,0,win->Width-1,win->Height-1); SetAPen(rp,1); text_at(rp,PAD_X,TITLE_Y,ISS_TITLE); draw_map(rp,depth,mx,my,mw,mh); draw_iss(rp,app,depth,mx,my,mw,mh); draw_panel(win,app); }
