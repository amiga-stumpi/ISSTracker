#include <exec/types.h>
#include <intuition/intuition.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <string.h>
#include "gui.h"
#include "draw.h"
#include "iss_api.h"
#include "citydb.h"
#include "astro.h"
#include "worldmask.h"
#include "config.h"
#define GID_UPDATE 1
#define GID_AUTO 2
#define GID_INFO 3
#define GID_TRAIL 4
#define GID_QUIT 5
#define BTN_W 58
#define BTN_H 16
static void button_layout(struct Window *win, WORD *by){ WORD mh; mh=(WORD)(win->Height-88); if(mh<80) mh=80; *by=(WORD)(18+mh+8); }
static int in_rect(WORD mx,WORD my,WORD x,WORD y,WORD w,WORD h){ return mx>=x && mx<=x+w && my>=y && my<=y+h; }
static void set_status(IssTrackerApp *app,UBYTE st,const char *txt){ UWORD i; app->status=st; for(i=0;txt[i]&&i+1<sizeof(app->status_text);i++) app->status_text[i]=txt[i]; app->status_text[i]=0; }
static void append_num(char *b,UWORD *p,LONG n){ char t[12]; WORD i; WORD j; if(n<0){ b[(*p)++]='-'; n=-n; } i=0; do{ t[i++]=(char)('0'+(n%10)); n/=10; }while(n&&i<11); for(j=i-1;j>=0;j--) b[(*p)++]=t[j]; b[*p]=0; }
static void update_now(struct Window *win,IssTrackerApp *app){ IssPosition p; char err[64]; err[0]=0; set_status(app,ISS_STATUS_LOADING,"LOADING"); strcpy(app->info_text,"Fetching ISS position..."); draw_panel(win,app); if(iss_fetch_now(&p,err,sizeof(err))==0){ app->current=p; iss_app_add_position(app,&p); app->light_state=astro_light_state(p.timestamp,p.lat_cd,p.lon_cd); app->surface_state=worldmask_classify(p.lat_cd,p.lon_cd); set_status(app,ISS_STATUS_ONLINE,"ONLINE"); strcpy(app->info_text,"ISS position updated"); draw_all(win,app); } else { set_status(app,ISS_STATUS_ERROR,"ERROR"); if(err[0]) strcpy(app->info_text,err); else strcpy(app->info_text,"Update failed"); draw_panel(win,app); } }
static void info_now(struct Window *win,IssTrackerApp *app){ const City *c; UWORD km; UWORD p; const char *s; if(!app->current.valid){ strcpy(app->info_text,"No ISS position yet"); draw_panel(win,app); return; } c=citydb_nearest(app->current.lat_cd,app->current.lon_cd,&km); strcpy(app->info_text,"Nearest: "); p=strlen(app->info_text); s=c->name; while(*s && p+1<sizeof(app->info_text)) app->info_text[p++]=*s++; app->info_text[p++]=' '; s=c->country; while(*s && p+1<sizeof(app->info_text)) app->info_text[p++]=*s++; app->info_text[p++]=' '; append_num(app->info_text,&p,km); if(p+3<sizeof(app->info_text)){ app->info_text[p++]='k'; app->info_text[p++]='m'; app->info_text[p]=0; } draw_panel(win,app); }
LONG gui_run(IssTrackerApp *app){ struct NewWindow nw; struct Window *win; ULONG sig; ULONG ticks; WORD done; memset(&nw,0,sizeof(nw)); nw.LeftEdge=app->win_left; nw.TopEdge=app->win_top; nw.Width=app->win_width; nw.Height=app->win_height; nw.DetailPen=0; nw.BlockPen=1; nw.IDCMPFlags=IDCMP_CLOSEWINDOW|IDCMP_REFRESHWINDOW|IDCMP_MOUSEBUTTONS|IDCMP_INTUITICKS|IDCMP_NEWSIZE; nw.Flags=WFLG_CLOSEGADGET|WFLG_DRAGBAR|WFLG_DEPTHGADGET|WFLG_SIZEGADGET|WFLG_ACTIVATE|WFLG_SMART_REFRESH; nw.Type=WBENCHSCREEN; nw.Title=(UBYTE*)ISS_TITLE; nw.MinWidth=320; nw.MinHeight=190; nw.MaxWidth=640; nw.MaxHeight=512; win=OpenWindow(&nw); if(!win){ nw.LeftEdge=0; nw.TopEdge=0; nw.Width=nw.MinWidth; nw.Height=nw.MinHeight; win=OpenWindow(&nw); if(!win) return -1; } draw_all(win,app); done=0; ticks=0; while(!done){ sig=Wait(1UL<<win->UserPort->mp_SigBit); if(sig&(1UL<<win->UserPort->mp_SigBit)){ struct IntuiMessage *msg; while((msg=(struct IntuiMessage*)GetMsg(win->UserPort))){ ULONG cls; WORD mx; WORD my; cls=msg->Class; mx=msg->MouseX; my=msg->MouseY; ReplyMsg((struct Message*)msg); if(cls==IDCMP_CLOSEWINDOW) done=1; else if(cls==IDCMP_REFRESHWINDOW || cls==IDCMP_NEWSIZE) draw_all(win,app); else if(cls==IDCMP_INTUITICKS){ ticks++; app->blink=(UBYTE)!app->blink; if(app->current.valid) draw_iss_blink(win,app); if(app->auto_update && ticks>300){ ticks=0; update_now(win,app); } } else if(cls==IDCMP_MOUSEBUTTONS){ WORD by; button_layout(win,&by); if(in_rect(mx,my,8,by,BTN_W,BTN_H)) update_now(win,app); else if(in_rect(mx,my,72,by,BTN_W,BTN_H)){ app->auto_update=(UBYTE)!app->auto_update; draw_panel(win,app); } else if(in_rect(mx,my,136,by,BTN_W,BTN_H)) info_now(win,app); else if(in_rect(mx,my,200,by,BTN_W,BTN_H)){ app->show_trail=(UBYTE)!app->show_trail; draw_all(win,app); } else if(in_rect(mx,my,264,by,BTN_W,BTN_H)) done=1; } } } } config_save_window(win->LeftEdge,win->TopEdge,win->Width,win->Height); CloseWindow(win); return 0; }
