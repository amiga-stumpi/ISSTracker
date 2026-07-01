#include <exec/types.h>
#include <intuition/intuition.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/dos.h>
#include <proto/diskfont.h>
#include <exec/memory.h>
#include <graphics/text.h>
#include <libraries/dos.h>
#include <devices/timer.h>
#include <clib/alib_protos.h>
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
#define FUNFACT_SHOW_TICKS 300U
#define FUNFACT_MIN_TICKS 3000U
#define FUNFACT_RANGE_TICKS 3001U
#define MENU_PROJECT 0
#define MENU_SETTINGS 1
#define MENU_HELP 2
#define ITEM_QUIT 0
#define ITEM_UPDATE_INTERVAL 0
#define ITEM_LANGUAGE 1
#define ITEM_COLORS 2
#define ITEM_FONT 3
#define ITEM_INFO 0
#define SUB_LANG_DE 0
#define SUB_LANG_EN 1
#define SUB_LANG_PL 2
static struct IntuiText mi_quit_text = { 0,1,JAM1, 0,1, 0, (UBYTE *)"Quit", 0 };
static struct MenuItem mi_quit = { 0, 0,0, 60,10, ITEMTEXT|ITEMENABLED|HIGHBOX, 0, (APTR)&mi_quit_text, 0, 0, 0, 0 };
static struct IntuiText mi_update_interval_text = { 0,1,JAM1, 0,1, 0, (UBYTE *)"Update Interval...", 0 };
static struct IntuiText mi_language_text = { 0,1,JAM1, 0,1, 0, (UBYTE *)"Sprache", 0 };
static struct IntuiText mi_lang_de_text = { 0,1,JAM1, 0,1, 0, (UBYTE *)"Deutsch", 0 };
static struct IntuiText mi_lang_en_text = { 0,1,JAM1, 0,1, 0, (UBYTE *)"English", 0 };
static struct IntuiText mi_lang_pl_text = { 0,1,JAM1, 0,1, 0, (UBYTE *)"Polski", 0 };
static struct MenuItem mi_lang_pl = { 0, 150,20, 76,10, ITEMTEXT|ITEMENABLED|HIGHBOX, 0, (APTR)&mi_lang_pl_text, 0, 0, 0, 0 };
static struct MenuItem mi_lang_en = { &mi_lang_pl, 150,10, 76,10, ITEMTEXT|ITEMENABLED|HIGHBOX, 0, (APTR)&mi_lang_en_text, 0, 0, 0, 0 };
static struct MenuItem mi_lang_de = { &mi_lang_en, 150,0, 76,10, ITEMTEXT|ITEMENABLED|HIGHBOX, 0, (APTR)&mi_lang_de_text, 0, 0, 0, 0 };
static struct IntuiText mi_colors_text = { 0,1,JAM1, 0,1, 0, (UBYTE *)"Colors...", 0 };
static struct IntuiText mi_font_text = { 0,1,JAM1, 0,1, 0, (UBYTE *)"Font...", 0 };
static struct MenuItem mi_font = { 0, 0,30, 160,10, ITEMTEXT|ITEMENABLED|HIGHBOX, 0, (APTR)&mi_font_text, 0, 0, 0, 0 };
static struct MenuItem mi_colors = { &mi_font, 0,20, 160,10, ITEMTEXT|ITEMENABLED|HIGHBOX, 0, (APTR)&mi_colors_text, 0, 0, 0, 0 };
static struct MenuItem mi_language = { &mi_colors, 0,10, 160,10, ITEMTEXT|ITEMENABLED|HIGHBOX, 0, (APTR)&mi_language_text, 0, 0, (APTR)&mi_lang_de, 0 };
static struct MenuItem mi_update_interval = { &mi_language, 0,0, 160,10, ITEMTEXT|ITEMENABLED|HIGHBOX, 0, (APTR)&mi_update_interval_text, 0, 0, 0, 0 };
static struct IntuiText mi_info_text = { 0,1,JAM1, 0,1, 0, (UBYTE *)"Info", 0 };
static struct MenuItem mi_info = { 0, 0,0, 60,10, ITEMTEXT|ITEMENABLED|HIGHBOX, 0, (APTR)&mi_info_text, 0, 0, 0, 0 };
static struct Menu menu_help = { 0, 204,0, 16,10, MENUENABLED, (UBYTE *)"?", &mi_info, 0,0,0,0 };
static struct Menu menu_settings = { &menu_help, 72,0, 132,10, MENUENABLED, (UBYTE *)"Settings", &mi_update_interval, 0,0,0,0 };
static struct Menu menu_project = { &menu_settings, 0,0, 72,10, MENUENABLED, (UBYTE *)"Projekt", &mi_quit, 0,0,0,0 };
struct Library *DiskfontBase = 0;

static struct Gadget main_update_gad;
static struct Gadget main_info_gad;

static struct TextAttr topaz8_attr = { (STRPTR)"topaz.font", 8, FS_NORMAL, FPF_ROMFONT };
static void set_menu_topaz_font(void)
{
    mi_quit_text.ITextFont=&topaz8_attr;
    mi_update_interval_text.ITextFont=&topaz8_attr;
    mi_language_text.ITextFont=&topaz8_attr;
    mi_lang_de_text.ITextFont=&topaz8_attr;
    mi_lang_en_text.ITextFont=&topaz8_attr;
    mi_lang_pl_text.ITextFont=&topaz8_attr;
    mi_colors_text.ITextFont=&topaz8_attr;
    mi_font_text.ITextFont=&topaz8_attr;
    mi_info_text.ITextFont=&topaz8_attr;
}
static void close_topaz8_font(void){}
static const char *txt_project(IssTrackerApp *app){ if(app->language==ISS_LANG_EN) return "Project"; if(app->language==ISS_LANG_PL) return "Projekt"; return "Projekt"; }
static const char *txt_quit(IssTrackerApp *app){ if(app->language==ISS_LANG_DE) return "Beenden"; if(app->language==ISS_LANG_PL) return "Zakoncz"; return "Quit"; }
static const char *txt_settings(IssTrackerApp *app){ if(app->language==ISS_LANG_DE) return "Einstellungen"; if(app->language==ISS_LANG_PL) return "Ustawienia"; return "Settings"; }
static const char *txt_interval(IssTrackerApp *app){ if(app->language==ISS_LANG_DE) return "Update Intervall..."; if(app->language==ISS_LANG_PL) return "Interwal..."; return "Update Interval..."; }
static const char *txt_language(IssTrackerApp *app){ if(app->language==ISS_LANG_DE) return "Sprachen"; if(app->language==ISS_LANG_PL) return "Jezyki"; return "Languages"; }
static const char *txt_colors(IssTrackerApp *app){ if(app->language==ISS_LANG_DE) return "Farben..."; if(app->language==ISS_LANG_PL) return "Kolory..."; return "Colors..."; }
static const char *txt_colors_title(IssTrackerApp *app){ if(app->language==ISS_LANG_DE) return "Farben"; if(app->language==ISS_LANG_PL) return "Kolory"; return "Colors"; }
static const char *txt_font(IssTrackerApp *app){ if(app->language==ISS_LANG_DE) return "Schrift..."; if(app->language==ISS_LANG_PL) return "Czcionka..."; return "Font..."; }
static const char *txt_font_title(IssTrackerApp *app){ if(app->language==ISS_LANG_DE) return "Schrift"; if(app->language==ISS_LANG_PL) return "Czcionka"; return "Font"; }
static const char *txt_info_menu(IssTrackerApp *app){ (void)app; return "Info"; }
static const char *txt_minutes(IssTrackerApp *app){ if(app->language==ISS_LANG_DE) return "Minuten:"; if(app->language==ISS_LANG_PL) return "Minuty:"; return "Minutes:"; }
static const char *txt_interval_title(IssTrackerApp *app){ if(app->language==ISS_LANG_DE) return "Update Intervall"; if(app->language==ISS_LANG_PL) return "Interwal"; return "Update Interval"; }
static const char *txt_interval_head(IssTrackerApp *app){ if(app->language==ISS_LANG_DE) return "Update Intervall Minuten"; if(app->language==ISS_LANG_PL) return "Interwal w minutach"; return "Update interval minutes"; }
static const char *txt_cancel(IssTrackerApp *app){ if(app->language==ISS_LANG_PL) return "Anuluj"; return "Cancel"; }
static const char *txt_info_fail(IssTrackerApp *app){ if(app->language==ISS_LANG_DE) return "Info Fenster fehlgeschlagen"; if(app->language==ISS_LANG_PL) return "Okno info blad"; return "Info window failed"; }
static const char *txt_interval_changed(IssTrackerApp *app){ if(app->language==ISS_LANG_DE) return "Update Intervall geaendert"; if(app->language==ISS_LANG_PL) return "Interwal zmieniony"; return "Update interval changed"; }
static const char *txt_interval_fail(IssTrackerApp *app){ if(app->language==ISS_LANG_DE) return "Intervall Fenster fehlgeschlagen"; if(app->language==ISS_LANG_PL) return "Okno interwalu blad"; return "Interval window failed"; }
static const char *txt_language_changed(IssTrackerApp *app){ if(app->language==ISS_LANG_DE) return "Sprache geaendert"; if(app->language==ISS_LANG_PL) return "Jezyk zmieniony"; return "Language changed"; }
static const char *txt_prepare_network(IssTrackerApp *app){ if(app->language==ISS_LANG_DE) return "Bereite Netzwerk vor..."; if(app->language==ISS_LANG_PL) return "Przygotowuje siec..."; return "Preparing network..."; }
static const char *txt_prepare_network_tls(IssTrackerApp *app){ if(app->language==ISS_LANG_DE) return "Bereite Netzwerk vor... AmiTLS13 erkannt, erweiterte Daten werden geladen..."; if(app->language==ISS_LANG_PL) return "Przygotowuje siec... AmiTLS13 wykryty, laduje dane rozszerzone..."; return "Preparing network... AmiTLS13 detected, loading extended data..."; }
static const char *txt_over_europe(IssTrackerApp *app){ if(app->language==ISS_LANG_DE) return "Hinweis: ISS ist ueber Europa"; if(app->language==ISS_LANG_PL) return "Info: ISS jest nad Europa"; return "Notice: ISS is over Europe"; }
static UBYTE iss_over_europe(const IssPosition *p){ return (p->valid && p->lat_cd>=3500 && p->lat_cd<=7200 && p->lon_cd>=-2500 && p->lon_cd<=4500) ? 1 : 0; }
static void apply_menu_texts(IssTrackerApp *app){ set_menu_topaz_font(); menu_project.MenuName=(UBYTE *)txt_project(app); mi_quit_text.IText=(STRPTR)txt_quit(app); menu_settings.MenuName=(UBYTE *)txt_settings(app); mi_update_interval_text.IText=(STRPTR)txt_interval(app); mi_language_text.IText=(STRPTR)txt_language(app); mi_colors_text.IText=(STRPTR)txt_colors(app); mi_font_text.IText=(STRPTR)txt_font(app); mi_info_text.IText=(STRPTR)txt_info_menu(app); }
static void refresh_menu_texts(struct Window *w, IssTrackerApp *app){ ClearMenuStrip(w); apply_menu_texts(app); SetMenuStrip(w,&menu_project); }
static void set_language_from_menu(struct Window *w, IssTrackerApp *app, UWORD sub){ if(sub==SUB_LANG_DE) app->language=ISS_LANG_DE; else if(sub==SUB_LANG_EN) app->language=ISS_LANG_EN; else if(sub==SUB_LANG_PL) app->language=ISS_LANG_PL; else return; refresh_menu_texts(w,app); strcpy(app->info_text,txt_language_changed(app)); draw_panel(w,app); }
static UWORD next_funfact_delay(IssTrackerApp *app){ app->funfact_seed=(app->funfact_seed*1103515245UL)+12345UL; return (UWORD)(FUNFACT_MIN_TICKS+(UWORD)((app->funfact_seed>>16)%FUNFACT_RANGE_TICKS)); }
static UWORD next_funfact_index(IssTrackerApp *app){ UWORD count; count=funfact_count(); if(count==0) return 0; app->funfact_seed=(app->funfact_seed*1103515245UL)+12345UL; return (UWORD)((app->funfact_seed>>16)%count); }
static void text_at(struct RastPort *rp, WORD x, WORD y, const char *s){ Move(rp,x,y); Text(rp,(STRPTR)s,strlen(s)); }
static void set_status(IssTrackerApp *app, UBYTE st, const char *txt){ UWORD i; app->status=st; for(i=0;txt[i]&&i+1<sizeof(app->status_text);i++) app->status_text[i]=txt[i]; app->status_text[i]=0; }
static void append_num(char *b, UWORD *p, LONG n){ char t[12]; WORD i; WORD j; if(n<0){ b[(*p)++]='-'; n=-n; } i=0; do{ t[i++]=(char)('0'+(n%10)); n/=10; }while(n&&i<11); for(j=i-1;j>=0;j--) b[(*p)++]=t[j]; b[*p]=0; }
static void append_str(char *b, UWORD *p, const char *s){ while(*s && *p<90) b[(*p)++]=*s++; b[*p]=0; }
static UWORD parse_minutes(const char *s){ ULONG v; v=0; while(*s==' ') s++; while(*s>='0' && *s<='9'){ v=(v*10)+(*s-'0'); if(v>120) return 120; s++; } if(v<2) v=2; if(v>120) v=120; return (UWORD)v; }
static void init_button_gadget(struct Gadget *g, struct IntuiText *t, struct Border *b, WORD *xy, WORD x, WORD y, WORD w, WORD h, UWORD id, const char *label)
{
    memset(g,0,sizeof(*g));
    memset(t,0,sizeof(*t));
    memset(b,0,sizeof(*b));
    xy[0]=0; xy[1]=0;
    xy[2]=(WORD)(w-1); xy[3]=0;
    xy[4]=(WORD)(w-1); xy[5]=(WORD)(h-1);
    xy[6]=0; xy[7]=(WORD)(h-1);
    xy[8]=0; xy[9]=0;
    b->LeftEdge=0;
    b->TopEdge=0;
    b->FrontPen=1;
    b->BackPen=0;
    b->DrawMode=JAM1;
    b->Count=5;
    b->XY=xy;
    t->FrontPen=1;
    t->BackPen=0;
    t->DrawMode=JAM1;
    t->LeftEdge=6;
    t->TopEdge=3;
    t->ITextFont=&topaz8_attr;
    t->IText=(UBYTE *)label;
    g->LeftEdge=x;
    g->TopEdge=y;
    g->Width=w;
    g->Height=h;
    g->Flags=GFLG_GADGHCOMP;
    g->Activation=GACT_RELVERIFY;
    g->GadgetType=GTYP_BOOLGADGET;
    g->GadgetRender=(APTR)b;
    g->GadgetText=t;
    g->GadgetID=id;
}
static void refresh_dialog_gadgets(struct Window *w, struct Gadget *first, UWORD count)
{
    RefreshGList(first,w,0,count);
}
static void init_hit_gadget(struct Gadget *g, WORD x, WORD y, WORD w, WORD h, UWORD id)
{
    memset(g,0,sizeof(*g));
    g->LeftEdge=x;
    g->TopEdge=y;
    g->Width=w;
    g->Height=h;
    g->Flags=0;
    g->Activation=GACT_RELVERIFY;
    g->GadgetType=GTYP_BOOLGADGET;
    g->GadgetID=id;
}
static void draw_interval_window(struct Window *w, IssTrackerApp *app, struct Gadget *g){ struct RastPort *rp; rp=w->RPort; SetDrMd(rp,JAM1); SetAPen(rp,0); RectFill(rp,0,0,w->Width-1,w->Height-1); SetAPen(rp,1); text_at(rp,10,18,txt_interval_head(app)); text_at(rp,10,40,txt_minutes(app)); SetAPen(rp,1); Move(rp,112,26); Draw(rp,162,26); Draw(rp,162,45); Draw(rp,112,45); Draw(rp,112,26); refresh_dialog_gadgets(w,g,3); }
static void update_now(struct Window *win, IssTrackerApp *app)
{
    IssPosition p;
    char err[64];
    UBYTE old_blink;
    err[0]=0;
    set_status(app,ISS_STATUS_LOADING,"LOADING");
    if(app->language==ISS_LANG_DE) strcpy(app->info_text,"ISS Position wird geladen..."); else if(app->language==ISS_LANG_PL) strcpy(app->info_text,"Pobieranie pozycji ISS..."); else strcpy(app->info_text,"Fetching ISS position...");
    draw_panel(win,app);
    if(iss_fetch_now(app,&p,err,sizeof(err))==0){
        if(app->current.valid){
            old_blink=app->blink;
            app->blink=0;
            draw_iss_blink(win,app);
            app->blink=old_blink;
        }
        app->current=p;
        iss_app_add_position(app,&p);
        if(app->enhanced_valid && app->visibility==0) app->light_state=ISS_LIGHT_DAY;
        else if(app->enhanced_valid && app->visibility==1) app->light_state=ISS_LIGHT_NIGHT;
        else app->light_state=astro_light_state(p.timestamp,p.lat_cd,p.lon_cd);
        app->surface_state=worldmask_classify(p.lat_cd,p.lon_cd);
        if(app->crew_fetch_state<2){
            if(iss_fetch_crew(app,err,sizeof(err))==0) app->crew_fetch_state=2;
            else app->crew_fetch_state=(app->crew_fetch_state==0)?1:2;
        }
        app->blink=1;
        set_status(app,ISS_STATUS_ONLINE,"ONLINE");
        if(iss_over_europe(&p) && !app->europe_alert_seen){
            app->europe_alert_seen=1;
            strcpy(app->info_text,txt_over_europe(app));
            app->status_page=0;
            DisplayBeep(win->WScreen);
        } else {
            if(app->language==ISS_LANG_DE) strcpy(app->info_text,"ISS Position aktualisiert"); else if(app->language==ISS_LANG_PL) strcpy(app->info_text,"Pozycja ISS zaktualizowana"); else strcpy(app->info_text,"ISS position updated");
            if(!app->funfact_active) app->status_page=0;
        }
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

static void draw_info_window(struct Window *w, struct Gadget *ok)
{
    struct RastPort *rp;
    rp=w->RPort;
    SetDrMd(rp,JAM1);
    SetAPen(rp,0);
    RectFill(rp,0,0,w->Width-1,w->Height-1);
    SetAPen(rp,1);
    text_at(rp,12,18,"ISSTracker live for Kick1.3");
    text_at(rp,12,32,"Version: v1.3.0");
    text_at(rp,12,46,"by Marcel Jaehne");
    text_at(rp,12,60,"(c) 2026");
    text_at(rp,12,76,"If you want to buy me a coffe,");
    text_at(rp,12,90,"send me a buck to:");
    text_at(rp,12,104,"https://paypal.me/mytubefree");
    refresh_dialog_gadgets(w,ok,1);
}
static void open_info_window(IssTrackerApp *app, struct Window *parent)
{
    struct NewWindow nw;
    struct Window *w;
    struct Gadget ok_gad;
    struct IntuiText ok_txt;
    struct Border ok_bord;
    WORD ok_xy[10];
    WORD done;
    memset(&nw,0,sizeof(nw));
    init_button_gadget(&ok_gad,&ok_txt,&ok_bord,ok_xy,160,118,50,16,1,"OK");
    nw.LeftEdge=(WORD)(parent->LeftEdge+24);
    nw.TopEdge=(WORD)(parent->TopEdge+24);
    nw.Width=300;
    nw.Height=150;
    nw.DetailPen=0;
    nw.BlockPen=1;
    nw.IDCMPFlags=IDCMP_CLOSEWINDOW|IDCMP_REFRESHWINDOW|IDCMP_GADGETUP;
    nw.Flags=WFLG_CLOSEGADGET|WFLG_DRAGBAR|WFLG_DEPTHGADGET|WFLG_ACTIVATE|WFLG_SMART_REFRESH;
    nw.FirstGadget=&ok_gad;
    nw.Type=WBENCHSCREEN;
    nw.Title=(UBYTE *)"Info";
    w=OpenWindow(&nw);
    if(!w){ strcpy(app->info_text,txt_info_fail(app)); draw_panel(parent,app); return; }
    draw_info_window(w,&ok_gad);
    done=0;
    while(!done){ ULONG sig; sig=Wait(1UL<<w->UserPort->mp_SigBit); if(sig&(1UL<<w->UserPort->mp_SigBit)){ struct IntuiMessage *msg; while((msg=(struct IntuiMessage *)GetMsg(w->UserPort))){ ULONG cls; APTR iaddr; cls=msg->Class; iaddr=msg->IAddress; ReplyMsg((struct Message *)msg); if(cls==IDCMP_CLOSEWINDOW) done=1; else if(cls==IDCMP_REFRESHWINDOW) draw_info_window(w,&ok_gad); else if(cls==IDCMP_GADGETUP && iaddr==&ok_gad) done=1; } } }
    CloseWindow(w);
}
static void open_interval_window(IssTrackerApp *app, struct Window *parent)
{
    struct NewWindow nw;
    struct Window *w;
    struct Gadget sg;
    struct Gadget ok_gad;
    struct Gadget cancel_gad;
    struct IntuiText ok_txt;
    struct IntuiText cancel_txt;
    struct Border ok_bord;
    struct Border cancel_bord;
    WORD ok_xy[10];
    WORD cancel_xy[10];
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
    si.NumChars=(WORD)strlen(buf);
    si.BufferPos=si.NumChars;
    si.DispPos=0;
    sg.LeftEdge=116;
    sg.TopEdge=30;
    sg.Width=42;
    sg.Height=12;
    sg.Flags=GFLG_GADGHBOX;
    sg.Activation=GACT_RELVERIFY|GACT_IMMEDIATE;
    sg.GadgetType=GTYP_STRGADGET;
    sg.SpecialInfo=(APTR)&si;
    init_button_gadget(&ok_gad,&ok_txt,&ok_bord,ok_xy,62,58,42,16,1,"OK");
    init_button_gadget(&cancel_gad,&cancel_txt,&cancel_bord,cancel_xy,116,58,62,16,2,txt_cancel(app));
    sg.NextGadget=&ok_gad;
    ok_gad.NextGadget=&cancel_gad;
    memset(&nw,0,sizeof(nw));
    nw.LeftEdge=(WORD)(parent->LeftEdge+24);
    nw.TopEdge=(WORD)(parent->TopEdge+24);
    nw.Width=220;
    nw.Height=90;
    nw.DetailPen=0;
    nw.BlockPen=1;
    nw.IDCMPFlags=IDCMP_CLOSEWINDOW|IDCMP_REFRESHWINDOW|IDCMP_GADGETUP;
    nw.Flags=WFLG_CLOSEGADGET|WFLG_DRAGBAR|WFLG_DEPTHGADGET|WFLG_ACTIVATE|WFLG_SMART_REFRESH;
    nw.FirstGadget=&sg;
    nw.Type=WBENCHSCREEN;
    nw.Title=(UBYTE *)txt_interval_title(app);
    w=OpenWindow(&nw);
    if(!w){ strcpy(app->info_text,txt_interval_fail(app)); draw_panel(parent,app); return; }
    draw_interval_window(w,app,&sg);
    ActivateGadget(&sg,w,0);
    done=0;
    while(!done){ ULONG sig; sig=Wait(1UL<<w->UserPort->mp_SigBit); if(sig&(1UL<<w->UserPort->mp_SigBit)){ struct IntuiMessage *msg; while((msg=(struct IntuiMessage *)GetMsg(w->UserPort))){ ULONG cls; APTR iaddr; cls=msg->Class; iaddr=msg->IAddress; ReplyMsg((struct Message *)msg); if(cls==IDCMP_CLOSEWINDOW) done=1; else if(cls==IDCMP_REFRESHWINDOW) draw_interval_window(w,app,&sg); else if(cls==IDCMP_GADGETUP){ if(iaddr==&ok_gad){ app->update_interval_min=parse_minutes(buf); strcpy(app->info_text,txt_interval_changed(app)); draw_panel(parent,app); done=1; } else if(iaddr==&cancel_gad) done=1; } } } }
    CloseWindow(w);
}
static UBYTE color_max_pen(struct Window *w){ UBYTE d; d=(UBYTE)w->WScreen->BitMap.Depth; if(d>=8) return 255; if(d<1) return 1; return (UBYTE)((1U<<d)-1U); }
static UBYTE *color_role_ptr(IssTrackerApp *app, UWORD role){ if(role==0) return &app->pen_water; if(role==1) return &app->pen_land; if(role==2) return &app->pen_border; if(role==3) return &app->pen_marker; if(role==4) return &app->pen_marker_fill; if(role==5) return &app->pen_trail; if(role==6) return &app->pen_text; return &app->pen_panel_bg; }
static UWORD color_role_count(void){ return 8; }
static const char *color_role_name(IssTrackerApp *app, UWORD role){ if(app->language==ISS_LANG_DE){ if(role==0) return "Wasser"; if(role==1) return "Land"; if(role==2) return "Kartengrenze"; if(role==3) return "ISS Kreuz"; if(role==4) return "ISS Punkt"; if(role==5) return "Spur"; if(role==6) return "Text"; return "Hintergrund"; } if(app->language==ISS_LANG_PL){ if(role==0) return "Woda"; if(role==1) return "Lad"; if(role==2) return "Granica mapy"; if(role==3) return "ISS krzyz"; if(role==4) return "ISS punkt"; if(role==5) return "Slad"; if(role==6) return "Tekst"; return "Tlo"; } if(role==0) return "Water"; if(role==1) return "Land"; if(role==2) return "Map border"; if(role==3) return "ISS cross"; if(role==4) return "ISS dot"; if(role==5) return "Trail"; if(role==6) return "Text"; return "Background"; }
static void sync_string_info(struct StringInfo *si, char *buf)
{
    WORD n;
    n=(WORD)strlen(buf);
    if(n>=si->MaxChars) n=(WORD)(si->MaxChars-1);
    if(n<0) n=0;
    si->NumChars=n;
    si->BufferPos=n;
    si->DispPos=0;
}
static void set_pen_buf(char *buf, UBYTE v){ UWORD p; memset(buf,0,8); p=0; append_num(buf,&p,v); }
static WORD color_role_at(WORD mx, WORD my){ WORD row; if(mx<10 || mx>230 || my<28 || my>=156) return -1; row=(WORD)((my-28)/16); if(row>=0 && row<(WORD)color_role_count()) return row; return -1; }
static void draw_color_window(struct Window *w, IssTrackerApp *app, struct Gadget *g, UWORD gadget_count, UWORD selected, char *buf)
{
    struct RastPort *rp; UWORD i; UBYTE *pv; UBYTE sel_pen; char line[96]; UWORD p; (void)buf; rp=w->RPort; sel_pen=*color_role_ptr(app,selected); SetDrMd(rp,JAM1); SetAPen(rp,0); RectFill(rp,0,0,w->Width-1,w->Height-1); SetAPen(rp,1); Move(rp,4,4); Draw(rp,w->Width-5,4); Draw(rp,w->Width-5,w->Height-5); Draw(rp,4,w->Height-5); Draw(rp,4,4); text_at(rp,10,14,txt_colors_title(app)); for(i=0;i<color_role_count();i++){ pv=color_role_ptr(app,i); p=0; append_str(line,&p,color_role_name(app,i)); line[p++]=' '; append_num(line,&p,*pv); if(selected==i){ SetAPen(rp,*pv); RectFill(rp,10,(WORD)(28+i*16),230,(WORD)(42+i*16)); SetAPen(rp,(*pv==0)?1:0); } else SetAPen(rp,1); text_at(rp,14,(WORD)(39+i*16),line); SetAPen(rp,*pv); RectFill(rp,204,(WORD)(31+i*16),224,(WORD)(39+i*16)); SetAPen(rp,1); Move(rp,204,(WORD)(31+i*16)); Draw(rp,224,(WORD)(31+i*16)); Draw(rp,224,(WORD)(39+i*16)); Draw(rp,204,(WORD)(39+i*16)); Draw(rp,204,(WORD)(31+i*16)); } SetAPen(rp,1); text_at(rp,12,184,"Pen:"); SetAPen(rp,sel_pen); RectFill(rp,248,154,288,170); SetAPen(rp,1); Move(rp,248,154); Draw(rp,288,154); Draw(rp,288,170); Draw(rp,248,170); Draw(rp,248,154); refresh_dialog_gadgets(w,g,gadget_count); }
static UBYTE parse_pen_value(const char *s, UBYTE maxp){ ULONG v; v=0; while(*s==' ') s++; while(*s>='0'&&*s<='9'){ v=(v*10)+(*s-'0'); if(v>maxp) return maxp; s++; } return (UBYTE)v; }
static void reset_colors(IssTrackerApp *app, UBYTE maxp)
{
    app->pen_panel_bg=0;
    app->pen_text=(maxp>=1)?1:0;
    if(maxp>=7){
        app->pen_water=4;
        app->pen_land=7;
        app->pen_border=1;
        app->pen_marker=2;
        app->pen_marker_fill=3;
        app->pen_trail=5;
    } else if(maxp>=3){
        app->pen_water=1;
        app->pen_land=2;
        app->pen_border=3;
        app->pen_marker=3;
        app->pen_marker_fill=1;
        app->pen_trail=3;
    } else {
        app->pen_water=0;
        app->pen_land=1;
        app->pen_border=1;
        app->pen_marker=1;
        app->pen_marker_fill=0;
        app->pen_trail=1;
    }
}
static UBYTE open_color_window(IssTrackerApp *app, struct Window *parent)
{
    struct NewWindow nw; struct Window *w; struct Gadget sg; struct Gadget minus_gad; struct Gadget plus_gad; struct Gadget set_gad; struct Gadget reset_gad; struct Gadget ok_gad; struct IntuiText minus_txt; struct IntuiText plus_txt; struct IntuiText set_txt; struct IntuiText reset_txt; struct IntuiText ok_txt; struct Border minus_bord; struct Border plus_bord; struct Border set_bord; struct Border reset_bord; struct Border ok_bord; WORD minus_xy[10]; WORD plus_xy[10]; WORD set_xy[10]; WORD reset_xy[10]; WORD ok_xy[10]; struct StringInfo si; char buf[8]; char undo[8]; UWORD selected; WORD done; UBYTE maxp; UBYTE dirty; memset(&sg,0,sizeof(sg)); memset(&si,0,sizeof(si)); memset(buf,0,sizeof(buf)); memset(undo,0,sizeof(undo)); selected=0; dirty=0; set_pen_buf(buf,*color_role_ptr(app,selected)); si.Buffer=(STRPTR)buf; si.UndoBuffer=(STRPTR)undo; si.MaxChars=4; si.LongInt=*color_role_ptr(app,selected); sync_string_info(&si,buf); sg.LeftEdge=48; sg.TopEdge=176; sg.Width=36; sg.Height=12; sg.Flags=GFLG_GADGHBOX; sg.Activation=GACT_RELVERIFY; sg.GadgetType=GTYP_STRGADGET; sg.SpecialInfo=(APTR)&si; init_button_gadget(&minus_gad,&minus_txt,&minus_bord,minus_xy,92,176,18,16,1,"-"); init_button_gadget(&plus_gad,&plus_txt,&plus_bord,plus_xy,114,176,18,16,2,"+"); init_button_gadget(&set_gad,&set_txt,&set_bord,set_xy,142,176,42,16,3,"Set"); init_button_gadget(&reset_gad,&reset_txt,&reset_bord,reset_xy,190,176,54,16,4,"Reset"); init_button_gadget(&ok_gad,&ok_txt,&ok_bord,ok_xy,250,176,42,16,5,"OK"); sg.NextGadget=&minus_gad; minus_gad.NextGadget=&plus_gad; plus_gad.NextGadget=&set_gad; set_gad.NextGadget=&reset_gad; reset_gad.NextGadget=&ok_gad; memset(&nw,0,sizeof(nw)); nw.LeftEdge=(WORD)(parent->LeftEdge+24); nw.TopEdge=(WORD)(parent->TopEdge+24); nw.Width=305; nw.Height=205; nw.DetailPen=0; nw.BlockPen=1; nw.IDCMPFlags=IDCMP_CLOSEWINDOW|IDCMP_MOUSEBUTTONS|IDCMP_REFRESHWINDOW|IDCMP_GADGETUP; nw.Flags=WFLG_CLOSEGADGET|WFLG_DRAGBAR|WFLG_DEPTHGADGET|WFLG_ACTIVATE|WFLG_SMART_REFRESH; nw.FirstGadget=&sg; nw.Type=WBENCHSCREEN; nw.Title=(UBYTE *)txt_colors_title(app); w=OpenWindow(&nw); if(!w){ strcpy(app->info_text,"Color window failed"); draw_panel(parent,app); return 0; } maxp=color_max_pen(parent); draw_color_window(w,app,&sg,6,selected,buf); done=0; while(!done){ ULONG sig; sig=Wait(1UL<<w->UserPort->mp_SigBit); if(sig&(1UL<<w->UserPort->mp_SigBit)){ struct IntuiMessage *msg; while((msg=(struct IntuiMessage *)GetMsg(w->UserPort))){ ULONG cls; UWORD code; APTR iaddr; WORD mx; WORD my; WORD role; UBYTE val; cls=msg->Class; code=msg->Code; iaddr=msg->IAddress; mx=msg->MouseX; my=msg->MouseY; ReplyMsg((struct Message *)msg); if(cls==IDCMP_CLOSEWINDOW) done=1; else if(cls==IDCMP_REFRESHWINDOW) draw_color_window(w,app,&sg,6,selected,buf); else if(cls==IDCMP_MOUSEBUTTONS && code==SELECTUP){ role=color_role_at(mx,my); if(role>=0 && role<(WORD)color_role_count()){ selected=(UWORD)role; set_pen_buf(buf,*color_role_ptr(app,selected)); sync_string_info(&si,buf); draw_color_window(w,app,&sg,6,selected,buf); } } else if(cls==IDCMP_GADGETUP){ if(iaddr==&minus_gad){ val=*color_role_ptr(app,selected); if(val>0) val--; *color_role_ptr(app,selected)=val; dirty=1; si.LongInt=val; set_pen_buf(buf,val); sync_string_info(&si,buf); draw_color_window(w,app,&sg,6,selected,buf); } else if(iaddr==&plus_gad){ val=*color_role_ptr(app,selected); if(val<maxp) val++; *color_role_ptr(app,selected)=val; dirty=1; si.LongInt=val; set_pen_buf(buf,val); sync_string_info(&si,buf); draw_color_window(w,app,&sg,6,selected,buf); } else if(iaddr==&set_gad){ val=parse_pen_value(buf,maxp); *color_role_ptr(app,selected)=val; dirty=1; si.LongInt=val; set_pen_buf(buf,val); sync_string_info(&si,buf); draw_color_window(w,app,&sg,6,selected,buf); } else if(iaddr==&reset_gad){ reset_colors(app,maxp); dirty=1; selected=0; set_pen_buf(buf,*color_role_ptr(app,selected)); sync_string_info(&si,buf); draw_color_window(w,app,&sg,6,selected,buf); } else if(iaddr==&ok_gad){ val=parse_pen_value(buf,maxp); *color_role_ptr(app,selected)=val; dirty=1; done=1; } } } } }
    CloseWindow(w); return dirty;
}

static void app_close_font(IssTrackerApp *app){ app->ui_font=0; }
static UBYTE app_open_font(IssTrackerApp *app){ struct TextAttr ta; struct TextFont *tf; if(!DiskfontBase) return 0; ta.ta_Name=(STRPTR)app->font_name; ta.ta_YSize=app->font_size; ta.ta_Style=FS_NORMAL; ta.ta_Flags=FPF_DISKFONT; tf=OpenDiskFont(&ta); if(!tf) return 0; app->ui_font=tf; return 1; }
static void apply_window_font(struct Window *w, IssTrackerApp *app){ if(app->ui_font) SetFont(w->RPort,app->ui_font); }
static void copy_font_name(char *dst, const char *src){ UWORD i; for(i=0;i<31 && src[i];i++) dst[i]=src[i]; dst[i]=0; }
static UBYTE has_font_suffix(const char *s){ UWORD n; n=strlen(s); if(n<5) return 0; return (s[n-5]=='.' && s[n-4]=='f' && s[n-3]=='o' && s[n-2]=='n' && s[n-1]=='t') ? 1 : 0; }
static UWORD scan_fonts(char names[][32], UWORD max)
{
    BPTR lock; struct FileInfoBlock *fib; UWORD count; lock=Lock((STRPTR)"FONTS:",ACCESS_READ); if(!lock) return 0; fib=(struct FileInfoBlock *)AllocMem(sizeof(struct FileInfoBlock),MEMF_CLEAR); if(!fib){ UnLock(lock); return 0; } count=0; if(Examine(lock,fib)){ while(count<max && ExNext(lock,fib)){ if(fib->fib_DirEntryType<0 && has_font_suffix(fib->fib_FileName)){ copy_font_name(names[count],fib->fib_FileName); count++; } } } FreeMem(fib,sizeof(struct FileInfoBlock)); UnLock(lock); return count;
}
static WORD font_row_at(WORD mx, WORD my){ WORD row; if(mx<10 || mx>210 || my<24 || my>=144) return -1; row=(WORD)((my-24)/12); if(row>=0 && row<10) return row; return -1; }
static UWORD parse_font_size(const char *s){ ULONG v; v=0; while(*s==' ') s++; while(*s>='0'&&*s<='9'){ v=(v*10)+(*s-'0'); if(v>32) return 32; s++; } if(v<6) v=6; if(v>32) v=32; return (UWORD)v; }
static void set_size_buf(char *buf, UWORD v){ UWORD p; memset(buf,0,8); p=0; append_num(buf,&p,v); }
static void draw_font_window(struct Window *w, IssTrackerApp *app, char names[][32], UWORD count, UWORD top, UWORD selected, struct Gadget *g, UWORD gadget_count)
{
    struct RastPort *rp; UWORD i; UWORD idx; rp=w->RPort; if(app->ui_font) SetFont(rp,app->ui_font); SetDrMd(rp,JAM1); SetAPen(rp,0); RectFill(rp,0,0,w->Width-1,w->Height-1); SetAPen(rp,1); Move(rp,4,4); Draw(rp,w->Width-5,4); Draw(rp,w->Width-5,w->Height-5); Draw(rp,4,w->Height-5); Draw(rp,4,4); text_at(rp,10,16,txt_font_title(app)); for(i=0;i<10;i++){ idx=(UWORD)(top+i); if(idx<count){ if(idx==selected){ SetAPen(rp,1); RectFill(rp,10,(WORD)(24+i*12),210,(WORD)(35+i*12)); SetAPen(rp,0); } else SetAPen(rp,1); text_at(rp,14,(WORD)(34+i*12),names[idx]); } } refresh_dialog_gadgets(w,g,gadget_count); }
static void open_font_window(IssTrackerApp *app, struct Window *parent)
{
    struct NewWindow nw; struct Window *w; struct Gadget size_gad; struct Gadget minus_gad; struct Gadget plus_gad; struct Gadget up_gad; struct Gadget down_gad; struct Gadget ok_gad; struct Gadget cancel_gad; struct IntuiText minus_txt; struct IntuiText plus_txt; struct IntuiText up_txt; struct IntuiText down_txt; struct IntuiText ok_txt; struct IntuiText cancel_txt; struct Border minus_bord; struct Border plus_bord; struct Border up_bord; struct Border down_bord; struct Border ok_bord; struct Border cancel_bord; WORD minus_xy[10]; WORD plus_xy[10]; WORD up_xy[10]; WORD down_xy[10]; WORD ok_xy[10]; WORD cancel_xy[10]; struct StringInfo si; char names[64][32]; char sizebuf[8]; char undo[8]; UWORD count; UWORD top; UWORD selected; UWORD i; WORD done; UBYTE changed; count=scan_fonts(names,64); if(count==0){ strcpy(app->info_text,"No fonts found"); draw_panel(parent,app); return; } selected=0; for(i=0;i<count;i++){ if(strcmp(names[i],app->font_name)==0){ selected=i; break; } } top=(selected>9)?(UWORD)(selected-9):0; set_size_buf(sizebuf,app->font_size); memset(&size_gad,0,sizeof(size_gad)); memset(&si,0,sizeof(si)); memset(undo,0,sizeof(undo)); si.Buffer=(STRPTR)sizebuf; si.UndoBuffer=(STRPTR)undo; si.MaxChars=4; si.LongInt=app->font_size; sync_string_info(&si,sizebuf); size_gad.LeftEdge=54; size_gad.TopEdge=148; size_gad.Width=46; size_gad.Height=12; size_gad.Flags=GFLG_GADGHBOX; size_gad.Activation=GACT_RELVERIFY; size_gad.GadgetType=GTYP_STRGADGET; size_gad.SpecialInfo=(APTR)&si; init_button_gadget(&minus_gad,&minus_txt,&minus_bord,minus_xy,108,148,18,16,1,"-"); init_button_gadget(&plus_gad,&plus_txt,&plus_bord,plus_xy,130,148,18,16,2,"+"); init_button_gadget(&up_gad,&up_txt,&up_bord,up_xy,218,34,50,16,3,"Up"); init_button_gadget(&down_gad,&down_txt,&down_bord,down_xy,218,56,50,16,4,"Down"); init_button_gadget(&ok_gad,&ok_txt,&ok_bord,ok_xy,160,148,42,16,5,"OK"); init_button_gadget(&cancel_gad,&cancel_txt,&cancel_bord,cancel_xy,210,148,62,16,6,txt_cancel(app)); size_gad.NextGadget=&minus_gad; minus_gad.NextGadget=&plus_gad; plus_gad.NextGadget=&up_gad; up_gad.NextGadget=&down_gad; down_gad.NextGadget=&ok_gad; ok_gad.NextGadget=&cancel_gad; memset(&nw,0,sizeof(nw)); nw.LeftEdge=(WORD)(parent->LeftEdge+30); nw.TopEdge=(WORD)(parent->TopEdge+30); nw.Width=285; nw.Height=182; nw.DetailPen=0; nw.BlockPen=1; nw.IDCMPFlags=IDCMP_CLOSEWINDOW|IDCMP_MOUSEBUTTONS|IDCMP_REFRESHWINDOW|IDCMP_GADGETUP; nw.Flags=WFLG_CLOSEGADGET|WFLG_DRAGBAR|WFLG_DEPTHGADGET|WFLG_ACTIVATE|WFLG_SMART_REFRESH; nw.FirstGadget=&size_gad; nw.Type=WBENCHSCREEN; nw.Title=(UBYTE *)txt_font_title(app); w=OpenWindow(&nw); if(!w){ strcpy(app->info_text,"Font window failed"); draw_panel(parent,app); return; } apply_window_font(w,app); changed=0; done=0; draw_font_window(w,app,names,count,top,selected,&size_gad,7); while(!done){ ULONG sig; sig=Wait(1UL<<w->UserPort->mp_SigBit); if(sig&(1UL<<w->UserPort->mp_SigBit)){ struct IntuiMessage *msg; while((msg=(struct IntuiMessage *)GetMsg(w->UserPort))){ ULONG cls; UWORD code; APTR iaddr; WORD mx; WORD my; WORD row; UWORD sz; cls=msg->Class; code=msg->Code; iaddr=msg->IAddress; mx=msg->MouseX; my=msg->MouseY; ReplyMsg((struct Message *)msg); if(cls==IDCMP_CLOSEWINDOW) done=1; else if(cls==IDCMP_REFRESHWINDOW) draw_font_window(w,app,names,count,top,selected,&size_gad,7); else if(cls==IDCMP_MOUSEBUTTONS && code==SELECTUP){ row=font_row_at(mx,my); if(row>=0 && (UWORD)(top+row)<count){ selected=(UWORD)(top+row); draw_font_window(w,app,names,count,top,selected,&size_gad,7); } } else if(cls==IDCMP_GADGETUP){ if(iaddr==&up_gad){ if(top>0) top--; draw_font_window(w,app,names,count,top,selected,&size_gad,7); } else if(iaddr==&down_gad){ if(top+10<count) top++; draw_font_window(w,app,names,count,top,selected,&size_gad,7); } else if(iaddr==&minus_gad){ sz=parse_font_size(sizebuf); if(sz>6) sz--; si.LongInt=sz; set_size_buf(sizebuf,sz); sync_string_info(&si,sizebuf); draw_font_window(w,app,names,count,top,selected,&size_gad,7); } else if(iaddr==&plus_gad){ sz=parse_font_size(sizebuf); if(sz<32) sz++; si.LongInt=sz; set_size_buf(sizebuf,sz); sync_string_info(&si,sizebuf); draw_font_window(w,app,names,count,top,selected,&size_gad,7); } else if(iaddr==&ok_gad){ char old_name[32]; UWORD old_size; copy_font_name(old_name,app->font_name); old_size=app->font_size; copy_font_name(app->font_name,names[selected]); app->font_size=parse_font_size(sizebuf); if(!app_open_font(app)){ copy_font_name(app->font_name,old_name); app->font_size=old_size; strcpy(app->info_text,"Font load failed"); } else { strcpy(app->info_text,"Font changed"); changed=1; } done=1; } else if(iaddr==&cancel_gad) done=1; } } } } CloseWindow(w); if(changed) draw_all(parent,app);
}


struct GuiTimer
{
    struct MsgPort *port;
    struct timerequest *req;
    UBYTE opened;
    UBYTE pending;
};

static UBYTE gui_timer_init(struct GuiTimer *gt)
{
    memset(gt,0,sizeof(*gt));
    gt->port=CreatePort(0,0);
    if(!gt->port) return 0;
    gt->req=(struct timerequest *)AllocMem(sizeof(struct timerequest),MEMF_CLEAR|MEMF_PUBLIC);
    if(!gt->req){ DeletePort(gt->port); gt->port=0; return 0; }
    gt->req->tr_node.io_Message.mn_ReplyPort=gt->port;
    if(OpenDevice((STRPTR)TIMERNAME,UNIT_MICROHZ,(struct IORequest *)gt->req,0)!=0){ FreeMem(gt->req,sizeof(struct timerequest)); DeletePort(gt->port); gt->req=0; gt->port=0; return 0; }
    gt->opened=1;
    return 1;
}

static void gui_timer_start(struct GuiTimer *gt)
{
    if(!gt || !gt->opened || gt->pending) return;
    gt->req->tr_node.io_Command=TR_ADDREQUEST;
    gt->req->tr_time.tv_secs=0;
    gt->req->tr_time.tv_micro=100000;
    SendIO((struct IORequest *)gt->req);
    gt->pending=1;
}

static void gui_timer_stop(struct GuiTimer *gt)
{
    if(!gt || !gt->opened || !gt->pending) return;
    if(!CheckIO((struct IORequest *)gt->req)) AbortIO((struct IORequest *)gt->req);
    WaitIO((struct IORequest *)gt->req);
    gt->pending=0;
}

static UBYTE gui_timer_drain(struct GuiTimer *gt)
{
    struct Message *msg;
    UBYTE got;
    got=0;
    if(!gt || !gt->opened) return 0;
    while((msg=GetMsg(gt->port))){ (void)msg; got=1; }
    if(got) gt->pending=0;
    return got;
}

static void gui_timer_cleanup(struct GuiTimer *gt)
{
    if(!gt) return;
    gui_timer_stop(gt);
    if(gt->opened) CloseDevice((struct IORequest *)gt->req);
    if(gt->req) FreeMem(gt->req,sizeof(struct timerequest));
    if(gt->port) DeletePort(gt->port);
    memset(gt,0,sizeof(*gt));
}

static void gui_tick(struct Window *win, IssTrackerApp *app, ULONG *auto_ticks, UWORD *blink_ticks, UWORD *status_ticks)
{
    (*auto_ticks)++;
    (*blink_ticks)++;
    (*status_ticks)++;
    if(*blink_ticks>=5){ *blink_ticks=0; app->blink=(UBYTE)!app->blink; if(app->current.valid) draw_iss_blink(win,app); }
    if(app->funfact_active){ app->funfact_ticks++; if(app->funfact_ticks>=FUNFACT_SHOW_TICKS){ app->funfact_active=0; app->funfact_ticks=0; app->funfact_next_ticks=next_funfact_delay(app); draw_panel(win,app); } }
    else if(app->funfact_next_ticks>0){ app->funfact_next_ticks--; if(app->funfact_next_ticks==0){ app->funfact_index=next_funfact_index(app); app->funfact_active=1; app->funfact_ticks=0; app->status_page=1; draw_panel(win,app); } }
    if((app->status_page==6 && *status_ticks>=100) || (app->status_page!=6 && *status_ticks>=50)){ *status_ticks=0; if(!app->funfact_active){ if(app->current.valid){ if(app->status_page<1 || app->status_page>=6) app->status_page=1; else app->status_page=(UBYTE)(app->status_page+1); } else app->status_page=0; draw_panel(win,app); } }
    if(*auto_ticks>=((ULONG)app->update_interval_min*AUTO_TICKS_PER_MIN)){ *auto_ticks=0; update_now(win,app); }
}


LONG gui_run(IssTrackerApp *app)
{
    struct NewWindow nw;
    struct Window *win;
    ULONG sig;
    ULONG auto_ticks;
    UWORD blink_ticks;
    UWORD status_ticks;
    struct GuiTimer timer;
    ULONG wait_mask;
    ULONG win_mask;
    ULONG timer_mask;
    WORD done;
    memset(&nw,0,sizeof(nw));
    nw.LeftEdge=app->win_left;
    nw.TopEdge=app->win_top;
    nw.Width=app->win_width;
    nw.Height=app->win_height;
    nw.DetailPen=0;
    nw.BlockPen=1;
    nw.IDCMPFlags=IDCMP_CLOSEWINDOW|IDCMP_REFRESHWINDOW|IDCMP_MOUSEBUTTONS|IDCMP_NEWSIZE|IDCMP_MENUPICK|IDCMP_GADGETUP;
    nw.Flags=WFLG_CLOSEGADGET|WFLG_DRAGBAR|WFLG_DEPTHGADGET|WFLG_SIZEGADGET|WFLG_ACTIVATE|WFLG_SMART_REFRESH;
    nw.Type=WBENCHSCREEN;
    nw.Title=(UBYTE*)ISS_TITLE;
    nw.MinWidth=320;
    nw.MinHeight=190;
    nw.MaxWidth=640;
    nw.MaxHeight=512;
    init_hit_gadget(&main_update_gad,0,0,BTN_W,BTN_H,1);
    init_hit_gadget(&main_info_gad,64,0,BTN_W,BTN_H,2);
    main_update_gad.NextGadget=&main_info_gad;
    nw.FirstGadget=&main_update_gad;
    win=OpenWindow(&nw);
    if(!win){ nw.LeftEdge=0; nw.TopEdge=0; nw.Width=nw.MinWidth; nw.Height=nw.MinHeight; win=OpenWindow(&nw); if(!win) return -1; }
    app->update_gadget=&main_update_gad;
    app->info_gadget=&main_info_gad;
    apply_menu_texts(app);
    DiskfontBase=OpenLibrary("diskfont.library",0);
    if(DiskfontBase) app_open_font(app);
    apply_window_font(win,app);
    SetMenuStrip(win,&menu_project);
    if(iss_https_probe(app)) strcpy(app->info_text,txt_prepare_network_tls(app));
    else strcpy(app->info_text,txt_prepare_network(app));
    app->status_page=0;
    draw_all(win,app);
    done=0;
    if(app->funfact_seed==0) app->funfact_seed=(ULONG)win ^ 0x13572468UL;
    if(app->funfact_next_ticks==0) app->funfact_next_ticks=next_funfact_delay(app);
    auto_ticks=((ULONG)app->update_interval_min*AUTO_TICKS_PER_MIN)-50;
    blink_ticks=0;
    status_ticks=0;
    if(gui_timer_init(&timer)){ gui_timer_start(&timer); timer_mask=1UL<<timer.port->mp_SigBit; } else timer_mask=0;
    win_mask=1UL<<win->UserPort->mp_SigBit;
    while(!done){
        wait_mask=win_mask|timer_mask;
        sig=Wait(wait_mask);
        if(timer_mask && (sig&timer_mask)){ if(gui_timer_drain(&timer)){ gui_tick(win,app,&auto_ticks,&blink_ticks,&status_ticks); gui_timer_start(&timer); } }
        if(sig&win_mask){
            struct IntuiMessage *msg;
            UBYTE redraw;
            redraw=0;
            while((msg=(struct IntuiMessage*)GetMsg(win->UserPort))){
                ULONG cls;
                UWORD code;
                APTR iaddr;
                cls=msg->Class;
                code=msg->Code;
                iaddr=msg->IAddress;
                ReplyMsg((struct Message*)msg);
                if(cls==IDCMP_CLOSEWINDOW) done=1;
                else if(cls==IDCMP_REFRESHWINDOW){ BeginRefresh(win); EndRefresh(win,FALSE); }
                else if(cls==IDCMP_NEWSIZE) redraw=1;
                else if(cls==IDCMP_GADGETUP && iaddr){ struct Gadget *gad; gad=(struct Gadget *)iaddr; if(gad==&main_update_gad){ update_now(win,app); auto_ticks=0; } else if(gad==&main_info_gad) info_now(win,app); }
                else if(cls==IDCMP_MENUPICK){ if(MENUNUM(code)==MENU_PROJECT && ITEMNUM(code)==ITEM_QUIT) done=1; else if(MENUNUM(code)==MENU_SETTINGS && ITEMNUM(code)==ITEM_UPDATE_INTERVAL) open_interval_window(app,win); else if(MENUNUM(code)==MENU_SETTINGS && ITEMNUM(code)==ITEM_LANGUAGE) set_language_from_menu(win,app,SUBNUM(code)); else if(MENUNUM(code)==MENU_SETTINGS && ITEMNUM(code)==ITEM_COLORS){ if(open_color_window(app,win)) redraw=1; } else if(MENUNUM(code)==MENU_SETTINGS && ITEMNUM(code)==ITEM_FONT) open_font_window(app,win); else if(MENUNUM(code)==MENU_HELP && ITEMNUM(code)==ITEM_INFO) open_info_window(app,win); }
            }
            if(redraw) draw_all(win,app);
        }
    }
    gui_timer_cleanup(&timer);
    app->win_left=win->LeftEdge;
    app->win_top=win->TopEdge;
    app->win_width=win->Width;
    app->win_height=win->Height;
    app->update_gadget=0;
    app->info_gadget=0;
    config_save(app);
    ClearMenuStrip(win);
    close_topaz8_font();
    CloseWindow(win);
    app_close_font(app);
    DiskfontBase=0;
    return 0;
}
