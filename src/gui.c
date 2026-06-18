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
#include "funfacts.h"
#define BTN_W 58
#define BTN_H 16
#define AUTO_TICKS_PER_MIN 600UL
#define FUNFACT_SHOW_TICKS 600U
#define FUNFACT_MIN_TICKS 3000U
#define FUNFACT_RANGE_TICKS 3001U
#define MENU_PROJECT 0
#define MENU_SETTINGS 1
#define MENU_HELP 2
#define ITEM_QUIT 0
#define ITEM_UPDATE_INTERVAL 0
#define ITEM_LANGUAGE 1
#define ITEM_INFO 0
static struct IntuiText mi_quit_text = { 0,1,JAM1, 0,1, 0, (UBYTE *)"Quit", 0 };
static struct MenuItem mi_quit = { 0, 0,0, 60,10, ITEMTEXT|ITEMENABLED|HIGHBOX, 0, (APTR)&mi_quit_text, 0, 0, 0, 0 };
static struct IntuiText mi_update_interval_text = { 0,1,JAM1, 0,1, 0, (UBYTE *)"Update Interval...", 0 };
static struct IntuiText mi_language_text = { 0,1,JAM1, 0,1, 0, (UBYTE *)"Sprache...", 0 };
static struct MenuItem mi_language = { 0, 0,10, 160,10, ITEMTEXT|ITEMENABLED|HIGHBOX, 0, (APTR)&mi_language_text, 0, 0, 0, 0 };
static struct MenuItem mi_update_interval = { &mi_language, 0,0, 160,10, ITEMTEXT|ITEMENABLED|HIGHBOX, 0, (APTR)&mi_update_interval_text, 0, 0, 0, 0 };
static struct IntuiText mi_info_text = { 0,1,JAM1, 0,1, 0, (UBYTE *)"Info", 0 };
static struct MenuItem mi_info = { 0, 0,0, 60,10, ITEMTEXT|ITEMENABLED|HIGHBOX, 0, (APTR)&mi_info_text, 0, 0, 0, 0 };
static struct Menu menu_help = { 0, 176,0, 16,10, MENUENABLED, (UBYTE *)"?", &mi_info, 0,0,0,0 };
static struct Menu menu_settings = { &menu_help, 72,0, 104,10, MENUENABLED, (UBYTE *)"Settings", &mi_update_interval, 0,0,0,0 };
static struct Menu menu_project = { &menu_settings, 0,0, 72,10, MENUENABLED, (UBYTE *)"Projekt", &mi_quit, 0,0,0,0 };
static const char *txt_project(IssTrackerApp *app){ if(app->language==ISS_LANG_EN) return "Project"; if(app->language==ISS_LANG_PL) return "Projekt"; return "Projekt"; }
static const char *txt_quit(IssTrackerApp *app){ if(app->language==ISS_LANG_PL) return "Zakoncz"; return "Quit"; }
static const char *txt_settings(IssTrackerApp *app){ if(app->language==ISS_LANG_DE) return "Einstellungen"; if(app->language==ISS_LANG_PL) return "Ustawienia"; return "Settings"; }
static const char *txt_interval(IssTrackerApp *app){ if(app->language==ISS_LANG_DE) return "Update Intervall..."; if(app->language==ISS_LANG_PL) return "Interwal..."; return "Update Interval..."; }
static const char *txt_language(IssTrackerApp *app){ if(app->language==ISS_LANG_DE) return "Sprache..."; if(app->language==ISS_LANG_PL) return "Jezyk..."; return "Language..."; }
static const char *txt_minutes(IssTrackerApp *app){ if(app->language==ISS_LANG_DE) return "Minuten:"; if(app->language==ISS_LANG_PL) return "Minuty:"; return "Minutes:"; }
static const char *txt_interval_title(IssTrackerApp *app){ if(app->language==ISS_LANG_DE) return "Update Intervall"; if(app->language==ISS_LANG_PL) return "Interwal"; return "Update Interval"; }
static const char *txt_interval_head(IssTrackerApp *app){ if(app->language==ISS_LANG_DE) return "Update Intervall Minuten"; if(app->language==ISS_LANG_PL) return "Interwal w minutach"; return "Update interval minutes"; }
static const char *txt_cancel(IssTrackerApp *app){ if(app->language==ISS_LANG_PL) return "Anuluj"; return "Cancel"; }
static const char *txt_info_fail(IssTrackerApp *app){ if(app->language==ISS_LANG_DE) return "Info Fenster fehlgeschlagen"; if(app->language==ISS_LANG_PL) return "Okno info blad"; return "Info window failed"; }
static const char *txt_interval_changed(IssTrackerApp *app){ if(app->language==ISS_LANG_DE) return "Update Intervall geaendert"; if(app->language==ISS_LANG_PL) return "Interwal zmieniony"; return "Update interval changed"; }
static const char *txt_interval_fail(IssTrackerApp *app){ if(app->language==ISS_LANG_DE) return "Intervall Fenster fehlgeschlagen"; if(app->language==ISS_LANG_PL) return "Okno interwalu blad"; return "Interval window failed"; }
static const char *txt_language_title(IssTrackerApp *app){ if(app->language==ISS_LANG_DE) return "Sprache"; if(app->language==ISS_LANG_PL) return "Jezyk"; return "Language"; }
static const char *txt_language_changed(IssTrackerApp *app){ if(app->language==ISS_LANG_DE) return "Sprache geaendert"; if(app->language==ISS_LANG_PL) return "Jezyk zmieniony"; return "Language changed"; }
static void apply_menu_texts(IssTrackerApp *app){ menu_project.MenuName=(UBYTE *)txt_project(app); mi_quit_text.IText=(STRPTR)txt_quit(app); menu_settings.MenuName=(UBYTE *)txt_settings(app); mi_update_interval_text.IText=(STRPTR)txt_interval(app); mi_language_text.IText=(STRPTR)txt_language(app); }
static UWORD next_funfact_delay(IssTrackerApp *app){ app->funfact_seed=(app->funfact_seed*1103515245UL)+12345UL; return (UWORD)(FUNFACT_MIN_TICKS+(UWORD)((app->funfact_seed>>16)%FUNFACT_RANGE_TICKS)); }
static UWORD next_funfact_index(IssTrackerApp *app){ UWORD count; count=funfact_count(); if(count==0) return 0; app->funfact_seed=(app->funfact_seed*1103515245UL)+12345UL; return (UWORD)((app->funfact_seed>>16)%count); }
static void button_layout(struct Window *win, WORD *by){ WORD mh; mh=(WORD)(win->Height-88); if(mh<80) mh=80; *by=(WORD)(18+mh+8); }
static int in_rect(WORD mx, WORD my, WORD x, WORD y, WORD w, WORD h){ return mx>=x && mx<=x+w && my>=y && my<=y+h; }
static void text_at(struct RastPort *rp, WORD x, WORD y, const char *s){ Move(rp,x,y); Text(rp,(STRPTR)s,strlen(s)); }
static void set_status(IssTrackerApp *app, UBYTE st, const char *txt){ UWORD i; app->status=st; for(i=0;txt[i]&&i+1<sizeof(app->status_text);i++) app->status_text[i]=txt[i]; app->status_text[i]=0; }
static void append_num(char *b, UWORD *p, LONG n){ char t[12]; WORD i; WORD j; if(n<0){ b[(*p)++]='-'; n=-n; } i=0; do{ t[i++]=(char)('0'+(n%10)); n/=10; }while(n&&i<11); for(j=i-1;j>=0;j--) b[(*p)++]=t[j]; b[*p]=0; }
static UWORD parse_minutes(const char *s){ ULONG v; v=0; while(*s==' ') s++; while(*s>='0' && *s<='9'){ v=(v*10)+(*s-'0'); if(v>120) return 120; s++; } if(v<1) v=1; if(v>120) v=120; return (UWORD)v; }
static void draw_small_button(struct RastPort *rp, WORD x, WORD y, WORD w, WORD h, const char *label){ SetAPen(rp,1); RectFill(rp,x,y,x+w,y+h); SetAPen(rp,0); Move(rp,x+6,y+11); Text(rp,(STRPTR)label,strlen(label)); SetAPen(rp,3); Move(rp,x,y); Draw(rp,x+w,y); Draw(rp,x+w,y+h); Draw(rp,x,y+h); Draw(rp,x,y); }
static void draw_interval_window(struct Window *w, IssTrackerApp *app, struct Gadget *g){ struct RastPort *rp; rp=w->RPort; SetDrMd(rp,JAM1); SetAPen(rp,0); RectFill(rp,0,0,w->Width-1,w->Height-1); SetAPen(rp,1); text_at(rp,10,18,txt_interval_head(app)); text_at(rp,10,40,txt_minutes(app)); SetAPen(rp,1); Move(rp,82,26); Draw(rp,132,26); Draw(rp,132,45); Draw(rp,82,45); Draw(rp,82,26); RefreshGList(g,w,0,1); draw_small_button(rp,48,58,42,16,"OK"); draw_small_button(rp,102,58,54,16,txt_cancel(app)); }
static void update_now(struct Window *win, IssTrackerApp *app)
{
    IssPosition p;
    char err[64];
    UBYTE old_blink;
    err[0]=0;
    set_status(app,ISS_STATUS_LOADING,"LOADING");
    if(app->language==ISS_LANG_DE) strcpy(app->info_text,"ISS Position wird geladen..."); else if(app->language==ISS_LANG_PL) strcpy(app->info_text,"Pobieranie pozycji ISS..."); else strcpy(app->info_text,"Fetching ISS position...");
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
        if(app->language==ISS_LANG_DE) strcpy(app->info_text,"ISS Position aktualisiert"); else if(app->language==ISS_LANG_PL) strcpy(app->info_text,"Pozycja ISS zaktualizowana"); else strcpy(app->info_text,"ISS position updated");
        app->status_page=0;
        draw_iss_blink(win,app);
        draw_panel(win,app);
    } else {
        set_status(app,ISS_STATUS_ERROR,"ERROR");
        if(err[0]) strcpy(app->info_text,err); else { if(app->language==ISS_LANG_DE) strcpy(app->info_text,"Update fehlgeschlagen"); else if(app->language==ISS_LANG_PL) strcpy(app->info_text,"Aktualizacja blad"); else strcpy(app->info_text,"Update failed"); }
        app->status_page=0;
        draw_panel(win,app);
    }
}
static void info_now(struct Window *win, IssTrackerApp *app)
{
    const City *c;
    UWORD km;
    UWORD p;
    const char *s;
    if(!app->current.valid){ if(app->language==ISS_LANG_DE) strcpy(app->info_text,"Noch keine ISS Position"); else if(app->language==ISS_LANG_PL) strcpy(app->info_text,"Brak pozycji ISS"); else strcpy(app->info_text,"No ISS position yet"); app->status_page=0; draw_panel(win,app); return; }
    c=citydb_nearest(app->current.lat_cd,app->current.lon_cd,&km);
    if(app->language==ISS_LANG_DE) strcpy(app->info_text,"Naechste Stadt: "); else if(app->language==ISS_LANG_PL) strcpy(app->info_text,"Najblizsze miasto: "); else strcpy(app->info_text,"Nearest: ");
    p=strlen(app->info_text);
    s=c->name; while(*s && p+1<sizeof(app->info_text)) app->info_text[p++]=*s++;
    app->info_text[p++]=' ';
    s=c->country; while(*s && p+1<sizeof(app->info_text)) app->info_text[p++]=*s++;
    app->info_text[p++]=' ';
    append_num(app->info_text,&p,km);
    if(p+3<sizeof(app->info_text)){ app->info_text[p++]='k'; app->info_text[p++]='m'; app->info_text[p]=0; }
    app->status_page=0;
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
    text_at(rp,12,76,"If you want to buy me a coffe,");
    text_at(rp,12,90,"send me a buck to:");
    text_at(rp,12,104,"https://paypal.me/mytubefree");
    draw_small_button(rp,160,118,50,16,"OK");
}
static void open_info_window(IssTrackerApp *app, struct Window *parent)
{
    struct NewWindow nw;
    struct Window *w;
    WORD done;
    memset(&nw,0,sizeof(nw));
    nw.LeftEdge=(WORD)(parent->LeftEdge+24);
    nw.TopEdge=(WORD)(parent->TopEdge+24);
    nw.Width=300;
    nw.Height=150;
    nw.DetailPen=0;
    nw.BlockPen=1;
    nw.IDCMPFlags=IDCMP_CLOSEWINDOW|IDCMP_MOUSEBUTTONS|IDCMP_REFRESHWINDOW;
    nw.Flags=WFLG_CLOSEGADGET|WFLG_DRAGBAR|WFLG_DEPTHGADGET|WFLG_ACTIVATE|WFLG_SMART_REFRESH;
    nw.Type=WBENCHSCREEN;
    nw.Title=(UBYTE *)"Info";
    w=OpenWindow(&nw);
    if(!w){ strcpy(app->info_text,txt_info_fail(app)); draw_panel(parent,app); return; }
    draw_info_window(w);
    done=0;
    while(!done){ ULONG sig; sig=Wait(1UL<<w->UserPort->mp_SigBit); if(sig&(1UL<<w->UserPort->mp_SigBit)){ struct IntuiMessage *msg; while((msg=(struct IntuiMessage *)GetMsg(w->UserPort))){ ULONG cls; WORD mx; WORD my; cls=msg->Class; mx=msg->MouseX; my=msg->MouseY; ReplyMsg((struct Message *)msg); if(cls==IDCMP_CLOSEWINDOW) done=1; else if(cls==IDCMP_REFRESHWINDOW) draw_info_window(w); else if(cls==IDCMP_MOUSEBUTTONS){ if(in_rect(mx,my,160,118,50,16)) done=1; } } } }
    CloseWindow(w);
}
static void draw_language_window(struct Window *w, IssTrackerApp *app)
{
    struct RastPort *rp;
    rp=w->RPort;
    SetDrMd(rp,JAM1);
    SetAPen(rp,0);
    RectFill(rp,0,0,w->Width-1,w->Height-1);
    SetAPen(rp,1);
    text_at(rp,12,18,txt_language_title(app));
    draw_small_button(rp,18,34,70,16,"Deutsch");
    draw_small_button(rp,98,34,70,16,"English");
    draw_small_button(rp,58,58,70,16,"Polski");
}
static void open_language_window(IssTrackerApp *app, struct Window *parent)
{
    struct NewWindow nw;
    struct Window *w;
    WORD done;
    memset(&nw,0,sizeof(nw));
    nw.LeftEdge=(WORD)(parent->LeftEdge+24);
    nw.TopEdge=(WORD)(parent->TopEdge+24);
    nw.Width=190;
    nw.Height=92;
    nw.DetailPen=0;
    nw.BlockPen=1;
    nw.IDCMPFlags=IDCMP_CLOSEWINDOW|IDCMP_MOUSEBUTTONS|IDCMP_REFRESHWINDOW;
    nw.Flags=WFLG_CLOSEGADGET|WFLG_DRAGBAR|WFLG_DEPTHGADGET|WFLG_ACTIVATE|WFLG_SMART_REFRESH;
    nw.Type=WBENCHSCREEN;
    nw.Title=(UBYTE *)txt_language_title(app);
    w=OpenWindow(&nw);
    if(!w){ strcpy(app->info_text,"Language window failed"); draw_panel(parent,app); return; }
    draw_language_window(w,app);
    done=0;
    while(!done){ ULONG sig; sig=Wait(1UL<<w->UserPort->mp_SigBit); if(sig&(1UL<<w->UserPort->mp_SigBit)){ struct IntuiMessage *msg; while((msg=(struct IntuiMessage *)GetMsg(w->UserPort))){ ULONG cls; WORD mx; WORD my; cls=msg->Class; mx=msg->MouseX; my=msg->MouseY; ReplyMsg((struct Message *)msg); if(cls==IDCMP_CLOSEWINDOW) done=1; else if(cls==IDCMP_REFRESHWINDOW) draw_language_window(w,app); else if(cls==IDCMP_MOUSEBUTTONS){ if(in_rect(mx,my,18,34,70,16)){ app->language=ISS_LANG_DE; done=1; } else if(in_rect(mx,my,98,34,70,16)){ app->language=ISS_LANG_EN; done=1; } else if(in_rect(mx,my,58,58,70,16)){ app->language=ISS_LANG_PL; done=1; } } } } }
    CloseWindow(w);
    apply_menu_texts(app);
    ClearMenuStrip(parent);
    SetMenuStrip(parent,&menu_project);
    strcpy(app->info_text,txt_language_changed(app));
    draw_panel(parent,app);
}
static void open_interval_window(IssTrackerApp *app, struct Window *parent)
{
    struct NewWindow nw;
    struct Window *w;
    struct Gadget sg;
    struct StringInfo si;
    char buf[8];
    char undo[8];
    UWORD p;
    WORD done;
    memset(&sg,0,sizeof(sg));
    memset(&si,0,sizeof(si));
    memset(buf,0,sizeof(buf));
    memset(undo,0,sizeof(undo));
    p=0;
    append_num(buf,&p,app->update_interval_min);
    si.Buffer=(STRPTR)buf;
    si.UndoBuffer=(STRPTR)undo;
    si.MaxChars=4;
    si.LongInt=app->update_interval_min;
    sg.LeftEdge=86;
    sg.TopEdge=30;
    sg.Width=42;
    sg.Height=12;
    sg.Flags=GFLG_GADGHBOX;
    sg.Activation=GACT_RELVERIFY|GACT_LONGINT;
    sg.GadgetType=GTYP_STRGADGET;
    sg.SpecialInfo=(APTR)&si;
    memset(&nw,0,sizeof(nw));
    nw.LeftEdge=(WORD)(parent->LeftEdge+24);
    nw.TopEdge=(WORD)(parent->TopEdge+24);
    nw.Width=184;
    nw.Height=90;
    nw.DetailPen=0;
    nw.BlockPen=1;
    nw.IDCMPFlags=IDCMP_CLOSEWINDOW|IDCMP_MOUSEBUTTONS|IDCMP_REFRESHWINDOW|IDCMP_GADGETUP;
    nw.Flags=WFLG_CLOSEGADGET|WFLG_DRAGBAR|WFLG_DEPTHGADGET|WFLG_ACTIVATE|WFLG_SMART_REFRESH;
    nw.FirstGadget=&sg;
    nw.Type=WBENCHSCREEN;
    nw.Title=(UBYTE *)txt_interval_title(app);
    w=OpenWindow(&nw);
    if(!w){ strcpy(app->info_text,txt_interval_fail(app)); draw_panel(parent,app); return; }
    draw_interval_window(w,app,&sg);
    done=0;
    while(!done){ ULONG sig; sig=Wait(1UL<<w->UserPort->mp_SigBit); if(sig&(1UL<<w->UserPort->mp_SigBit)){ struct IntuiMessage *msg; while((msg=(struct IntuiMessage *)GetMsg(w->UserPort))){ ULONG cls; WORD mx; WORD my; cls=msg->Class; mx=msg->MouseX; my=msg->MouseY; ReplyMsg((struct Message *)msg); if(cls==IDCMP_CLOSEWINDOW) done=1; else if(cls==IDCMP_REFRESHWINDOW) draw_interval_window(w,app,&sg); else if(cls==IDCMP_MOUSEBUTTONS){ if(in_rect(mx,my,48,58,42,16)){ app->update_interval_min=parse_minutes(buf); strcpy(app->info_text,txt_interval_changed(app)); draw_panel(parent,app); done=1; } else if(in_rect(mx,my,102,58,54,16)) done=1; } } } }
    CloseWindow(w);
}
LONG gui_run(IssTrackerApp *app)
{
    struct NewWindow nw;
    struct Window *win;
    ULONG sig;
    ULONG auto_ticks;
    UWORD blink_ticks;
    UWORD status_ticks;
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
    apply_menu_texts(app);
    SetMenuStrip(win,&menu_project);
    draw_all(win,app);
    done=0;
    if(app->funfact_seed==0) app->funfact_seed=(ULONG)win ^ 0x13572468UL;
    if(app->funfact_next_ticks==0) app->funfact_next_ticks=next_funfact_delay(app);
    auto_ticks=((ULONG)app->update_interval_min*AUTO_TICKS_PER_MIN)-1;
    blink_ticks=0;
    status_ticks=0;
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
                else if(cls==IDCMP_MENUPICK){ if(MENUNUM(code)==MENU_PROJECT && ITEMNUM(code)==ITEM_QUIT) done=1; else if(MENUNUM(code)==MENU_SETTINGS && ITEMNUM(code)==ITEM_UPDATE_INTERVAL) open_interval_window(app,win); else if(MENUNUM(code)==MENU_SETTINGS && ITEMNUM(code)==ITEM_LANGUAGE) open_language_window(app,win); else if(MENUNUM(code)==MENU_HELP && ITEMNUM(code)==ITEM_INFO) open_info_window(app,win); }
                else if(cls==IDCMP_INTUITICKS){
                    auto_ticks++;
                    blink_ticks++;
                    status_ticks++;
                    if(blink_ticks>=5){ blink_ticks=0; app->blink=(UBYTE)!app->blink; if(app->current.valid) draw_iss_blink(win,app); }
                    if(app->funfact_active){ app->funfact_ticks++; if(app->funfact_ticks>=FUNFACT_SHOW_TICKS){ app->funfact_active=0; app->funfact_ticks=0; app->funfact_next_ticks=next_funfact_delay(app); draw_panel(win,app); } }
                    else if(app->funfact_next_ticks>0){ app->funfact_next_ticks--; if(app->funfact_next_ticks==0){ app->funfact_index=next_funfact_index(app); app->funfact_active=1; app->funfact_ticks=0; app->status_page=1; draw_panel(win,app); } }
                    if(status_ticks>=50){ status_ticks=0; if(!app->funfact_active){ if(app->current.valid){ if(app->status_page<1 || app->status_page>=3) app->status_page=1; else app->status_page=(UBYTE)(app->status_page+1); } else app->status_page=0; draw_panel(win,app); } }
                    if(auto_ticks>=((ULONG)app->update_interval_min*AUTO_TICKS_PER_MIN)){ auto_ticks=0; update_now(win,app); }
                } else if(cls==IDCMP_MOUSEBUTTONS){
                    WORD by;
                    button_layout(win,&by);
                    if(in_rect(mx,my,8,by,BTN_W,BTN_H)){ auto_ticks=0; update_now(win,app); }
                    else if(in_rect(mx,my,72,by,BTN_W,BTN_H)) info_now(win,app);
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
