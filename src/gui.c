#include <exec/types.h>
#include <intuition/intuition.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <string.h>
#include "gui.h"
#include "draw.h"
#include "iss_api.h"
#include "citydb.h"
#include "astro.h"
#include "worldmask.h"
#include "config.h"
#define BTN_W 58
#define BTN_H 16
#define AUTO_TICKS_PER_MIN 600UL
#define MENU_SETTINGS 0
#define MENU_HELP 1
#define ITEM_UPDATE_INTERVAL 0
#define ITEM_INFO 0
static struct IntuiText mi_update_interval_text = { 0,1,JAM1, 0,1, 0, (UBYTE *)"Update Interval...", 0 };
static struct MenuItem mi_update_interval = { 0, 0,0, 160,10, ITEMTEXT|ITEMENABLED|HIGHBOX, 0, (APTR)&mi_update_interval_text, 0, 0, 0, 0 };
static struct IntuiText mi_info_text = { 0,1,JAM1, 0,1, 0, (UBYTE *)"Info", 0 };
static struct MenuItem mi_info = { 0, 0,0, 60,10, ITEMTEXT|ITEMENABLED|HIGHBOX, 0, (APTR)&mi_info_text, 0, 0, 0, 0 };
static struct Menu menu_help = { 0, 78,0, 16,10, MENUENABLED, (UBYTE *)"?", &mi_info, 0,0,0,0 };
static struct Menu menu_settings = { &menu_help, 0,0, 70,10, MENUENABLED, (UBYTE *)"Settings", &mi_update_interval, 0,0,0,0 };
static void button_layout(struct Window *win, WORD *by){ WORD mh; mh=(WORD)(win->Height-88); if(mh<80) mh=80; *by=(WORD)(18+mh+8); }
static int in_rect(WORD mx, WORD my, WORD x, WORD y, WORD w, WORD h){ return mx>=x && mx<=x+w && my>=y && my<=y+h; }
static void text_at(struct RastPort *rp, WORD x, WORD y, const char *s){ Move(rp,x,y); Text(rp,(STRPTR)s,strlen(s)); }
static void set_status(IssTrackerApp *app, UBYTE st, const char *txt){ UWORD i; app->status=st; for(i=0;txt[i]&&i+1<sizeof(app->status_text);i++) app->status_text[i]=txt[i]; app->status_text[i]=0; }
static void append_num(char *b, UWORD *p, LONG n){ char t[12]; WORD i; WORD j; if(n<0){ b[(*p)++]='-'; n=-n; } i=0; do{ t[i++]=(char)('0'+(n%10)); n/=10; }while(n&&i<11); for(j=i-1;j>=0;j--) b[(*p)++]=t[j]; b[*p]=0; }
static void draw_small_button(struct RastPort *rp, WORD x, WORD y, WORD w, WORD h, const char *label){ SetAPen(rp,1); RectFill(rp,x,y,x+w,y+h); SetAPen(rp,0); Move(rp,x+6,y+11); Text(rp,(STRPTR)label,strlen(label)); SetAPen(rp,3); Move(rp,x,y); Draw(rp,x+w,y); Draw(rp,x+w,y+h); Draw(rp,x,y+h); Draw(rp,x,y); }
static void draw_interval_window(struct Window *w, UWORD value){ struct RastPort *rp; char num[8]; UWORD p; rp=w->RPort; SetDrMd(rp,JAM1); SetAPen(rp,0); RectFill(rp,0,0,w->Width-1,w->Height-1); SetAPen(rp,1); text_at(rp,10,18,"Update interval (minutes)"); p=0; append_num(num,&p,value); SetAPen(rp,1); text_at(rp,96,40,num); draw_small_button(rp,24,55,34,16,"-"); draw_small_button(rp,66,55,34,16,"+"); draw_small_button(rp,118,55,42,16,"OK"); draw_small_button(rp,168,55,54,16,"Cancel"); }
static void update_now(struct Window *win, IssTrackerApp *app)
{
    IssPosition p;
    char err[64];
    UBYTE old_blink;
    err[0]=0;
    set_status(app,ISS_STATUS_LOADING,"LOADING");
    strcpy(app->info_text,"Fetching ISS position...");
    draw_panel(win,app);
    if(iss_fetch_now(&p,err,sizeof(err))==0){
        if(app->current.valid){
            old_blink=app->blink;
            app->blink=0;
            draw_iss_blink(win,app);
            app->blink=old_blink;
        }
        app->current=p;
        iss_app_add_position(app,&p);
        app->light_state=astro_light_state(p.timestamp,p.lat_cd,p.lon_cd);
        app->surface_state=worldmask_classify(p.lat_cd,p.lon_cd);
        app->blink=1;
        set_status(app,ISS_STATUS_ONLINE,"ONLINE");
        strcpy(app->info_text,"ISS position updated");
        draw_iss_blink(win,app);
        draw_panel(win,app);
    } else {
        set_status(app,ISS_STATUS_ERROR,"ERROR");
        if(err[0]) strcpy(app->info_text,err); else strcpy(app->info_text,"Update failed");
        draw_panel(win,app);
    }
}
static void info_now(struct Window *win, IssTrackerApp *app)
{
    const City *c;
    UWORD km;
    UWORD p;
    const char *s;
    if(!app->current.valid){ strcpy(app->info_text,"No ISS position yet"); draw_panel(win,app); return; }
    c=citydb_nearest(app->current.lat_cd,app->current.lon_cd,&km);
    strcpy(app->info_text,"Nearest: ");
    p=strlen(app->info_text);
    s=c->name; while(*s && p+1<sizeof(app->info_text)) app->info_text[p++]=*s++;
    app->info_text[p++]=' ';
    s=c->country; while(*s && p+1<sizeof(app->info_text)) app->info_text[p++]=*s++;
    app->info_text[p++]=' ';
    append_num(app->info_text,&p,km);
    if(p+3<sizeof(app->info_text)){ app->info_text[p++]='k'; app->info_text[p++]='m'; app->info_text[p]=0; }
    draw_panel(win,app);
}

static void draw_info_window(struct Window *w)
{
    struct RastPort *rp;
    rp=w->RPort;
    SetDrMd(rp,JAM1);
    SetAPen(rp,0);
    RectFill(rp,0,0,w->Width-1,w->Height-1);
    SetAPen(rp,1);
    text_at(rp,12,18,"ISS Tracker for Kick1.3");
    text_at(rp,12,32,"Version: v1.0");
    text_at(rp,12,46,"by Marcel Jaehne");
    text_at(rp,12,60,"(c) 2026");
    draw_small_button(rp,88,78,50,16,"OK");
}
static void open_info_window(IssTrackerApp *app, struct Window *parent)
{
    struct NewWindow nw;
    struct Window *w;
    WORD done;
    memset(&nw,0,sizeof(nw));
    nw.LeftEdge=(WORD)(parent->LeftEdge+24);
    nw.TopEdge=(WORD)(parent->TopEdge+24);
    nw.Width=230;
    nw.Height=110;
    nw.DetailPen=0;
    nw.BlockPen=1;
    nw.IDCMPFlags=IDCMP_CLOSEWINDOW|IDCMP_MOUSEBUTTONS|IDCMP_REFRESHWINDOW;
    nw.Flags=WFLG_CLOSEGADGET|WFLG_DRAGBAR|WFLG_DEPTHGADGET|WFLG_ACTIVATE|WFLG_SMART_REFRESH;
    nw.Type=WBENCHSCREEN;
    nw.Title=(UBYTE *)"Info";
    w=OpenWindow(&nw);
    if(!w){ strcpy(app->info_text,"Info window failed"); draw_panel(parent,app); return; }
    draw_info_window(w);
    done=0;
    while(!done){ ULONG sig; sig=Wait(1UL<<w->UserPort->mp_SigBit); if(sig&(1UL<<w->UserPort->mp_SigBit)){ struct IntuiMessage *msg; while((msg=(struct IntuiMessage *)GetMsg(w->UserPort))){ ULONG cls; WORD mx; WORD my; cls=msg->Class; mx=msg->MouseX; my=msg->MouseY; ReplyMsg((struct Message *)msg); if(cls==IDCMP_CLOSEWINDOW) done=1; else if(cls==IDCMP_REFRESHWINDOW) draw_info_window(w); else if(cls==IDCMP_MOUSEBUTTONS){ if(in_rect(mx,my,88,78,50,16)) done=1; } } } }
    CloseWindow(w);
}
static void open_interval_window(IssTrackerApp *app, struct Window *parent)
{
    struct NewWindow nw;
    struct Window *w;
    WORD done;
    UWORD val;
    memset(&nw,0,sizeof(nw));
    nw.LeftEdge=(WORD)(parent->LeftEdge+24);
    nw.TopEdge=(WORD)(parent->TopEdge+24);
    nw.Width=236;
    nw.Height=86;
    nw.DetailPen=0;
    nw.BlockPen=1;
    nw.IDCMPFlags=IDCMP_CLOSEWINDOW|IDCMP_MOUSEBUTTONS|IDCMP_REFRESHWINDOW;
    nw.Flags=WFLG_CLOSEGADGET|WFLG_DRAGBAR|WFLG_DEPTHGADGET|WFLG_ACTIVATE|WFLG_SMART_REFRESH;
    nw.Type=WBENCHSCREEN;
    nw.Title=(UBYTE *)"Update Interval";
    w=OpenWindow(&nw);
    if(!w){ strcpy(app->info_text,"Interval window failed"); draw_panel(parent,app); return; }
    val=app->update_interval_min;
    draw_interval_window(w,val);
    done=0;
    while(!done){ ULONG sig; sig=Wait(1UL<<w->UserPort->mp_SigBit); if(sig&(1UL<<w->UserPort->mp_SigBit)){ struct IntuiMessage *msg; while((msg=(struct IntuiMessage *)GetMsg(w->UserPort))){ ULONG cls; WORD mx; WORD my; cls=msg->Class; mx=msg->MouseX; my=msg->MouseY; ReplyMsg((struct Message *)msg); if(cls==IDCMP_CLOSEWINDOW) done=1; else if(cls==IDCMP_REFRESHWINDOW) draw_interval_window(w,val); else if(cls==IDCMP_MOUSEBUTTONS){ if(in_rect(mx,my,24,55,34,16)){ if(val>5) val=(UWORD)(val-5); draw_interval_window(w,val); } else if(in_rect(mx,my,66,55,34,16)){ if(val<120) val=(UWORD)(val+5); draw_interval_window(w,val); } else if(in_rect(mx,my,118,55,42,16)){ app->update_interval_min=val; strcpy(app->info_text,"Update interval changed"); draw_panel(parent,app); done=1; } else if(in_rect(mx,my,168,55,54,16)) done=1; } } } }
    CloseWindow(w);
}
LONG gui_run(IssTrackerApp *app)
{
    struct NewWindow nw;
    struct Window *win;
    ULONG sig;
    ULONG auto_ticks;
    UWORD blink_ticks;
    WORD done;
    memset(&nw,0,sizeof(nw));
    nw.LeftEdge=app->win_left;
    nw.TopEdge=app->win_top;
    nw.Width=app->win_width;
    nw.Height=app->win_height;
    nw.DetailPen=0;
    nw.BlockPen=1;
    nw.IDCMPFlags=IDCMP_CLOSEWINDOW|IDCMP_REFRESHWINDOW|IDCMP_MOUSEBUTTONS|IDCMP_INTUITICKS|IDCMP_NEWSIZE|IDCMP_MENUPICK;
    nw.Flags=WFLG_CLOSEGADGET|WFLG_DRAGBAR|WFLG_DEPTHGADGET|WFLG_SIZEGADGET|WFLG_ACTIVATE|WFLG_SMART_REFRESH;
    nw.Type=WBENCHSCREEN;
    nw.Title=(UBYTE*)ISS_TITLE;
    nw.MinWidth=320;
    nw.MinHeight=190;
    nw.MaxWidth=640;
    nw.MaxHeight=512;
    win=OpenWindow(&nw);
    if(!win){ nw.LeftEdge=0; nw.TopEdge=0; nw.Width=nw.MinWidth; nw.Height=nw.MinHeight; win=OpenWindow(&nw); if(!win) return -1; }
    SetMenuStrip(win,&menu_settings);
    draw_all(win,app);
    done=0;
    auto_ticks=0;
    blink_ticks=0;
    while(!done){
        sig=Wait(1UL<<win->UserPort->mp_SigBit);
        if(sig&(1UL<<win->UserPort->mp_SigBit)){
            struct IntuiMessage *msg;
            while((msg=(struct IntuiMessage*)GetMsg(win->UserPort))){
                ULONG cls;
                UWORD code;
                WORD mx;
                WORD my;
                cls=msg->Class;
                code=msg->Code;
                mx=msg->MouseX;
                my=msg->MouseY;
                ReplyMsg((struct Message*)msg);
                if(cls==IDCMP_CLOSEWINDOW) done=1;
                else if(cls==IDCMP_REFRESHWINDOW || cls==IDCMP_NEWSIZE) draw_all(win,app);
                else if(cls==IDCMP_MENUPICK){ if(MENUNUM(code)==MENU_SETTINGS && ITEMNUM(code)==ITEM_UPDATE_INTERVAL) open_interval_window(app,win); else if(MENUNUM(code)==MENU_HELP && ITEMNUM(code)==ITEM_INFO) open_info_window(app,win); }
                else if(cls==IDCMP_INTUITICKS){
                    auto_ticks++;
                    blink_ticks++;
                    if(blink_ticks>=5){ blink_ticks=0; app->blink=(UBYTE)!app->blink; if(app->current.valid) draw_iss_blink(win,app); }
                    if(app->auto_update && auto_ticks>=((ULONG)app->update_interval_min*AUTO_TICKS_PER_MIN)){ auto_ticks=0; update_now(win,app); }
                } else if(cls==IDCMP_MOUSEBUTTONS){
                    WORD by;
                    button_layout(win,&by);
                    if(in_rect(mx,my,8,by,BTN_W,BTN_H)) update_now(win,app);
                    else if(in_rect(mx,my,72,by,BTN_W,BTN_H)){ app->auto_update=(UBYTE)!app->auto_update; auto_ticks=0; draw_panel(win,app); }
                    else if(in_rect(mx,my,136,by,BTN_W,BTN_H)) info_now(win,app);
                    else if(in_rect(mx,my,200,by,BTN_W,BTN_H)){ app->show_trail=(UBYTE)!app->show_trail; draw_all(win,app); }
                    else if(in_rect(mx,my,264,by,BTN_W,BTN_H)) done=1;
                }
            }
        }
    }
    ClearMenuStrip(win);
    app->win_left=win->LeftEdge;
    app->win_top=win->TopEdge;
    app->win_width=win->Width;
    app->win_height=win->Height;
    config_save(app);
    CloseWindow(win);
    return 0;
}
