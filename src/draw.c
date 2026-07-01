#include <proto/graphics.h>
#include <proto/intuition.h>
#include <graphics/text.h>
#include <string.h>
#include "draw.h"
#include "map_projection.h"
#include "worldmask.h"
#include "map_image.h"
#include "funfacts.h"
#define PAD_X 8
#define MAP_TOP 6
#define BTN_W 58
#define BTN_H 16
#define STATUS_GAP 13
#define MIN_MAP_W 300
#define MIN_MAP_H 100
#define MARK_R 7

static void text_at(struct RastPort *rp, WORD x, WORD y, const char *s){ Move(rp,x,y); Text(rp,(STRPTR)s,strlen(s)); }
static void text_at_n(struct RastPort *rp, WORD x, WORD y, const char *s, UWORD n){ Move(rp,x,y); Text(rp,(STRPTR)s,n); }
static const char *status_name(IssTrackerApp *app){ if(app->language==ISS_LANG_DE){ if(app->status==ISS_STATUS_ONLINE) return "Online"; if(app->status==ISS_STATUS_LOADING) return "Laden"; if(app->status==ISS_STATUS_ERROR) return "Fehler"; return "Offline"; } if(app->language==ISS_LANG_PL){ if(app->status==ISS_STATUS_ONLINE) return "Online"; if(app->status==ISS_STATUS_LOADING) return "Ladowanie"; if(app->status==ISS_STATUS_ERROR) return "Blad"; return "Offline"; } if(app->status==ISS_STATUS_ONLINE) return "Online"; if(app->status==ISS_STATUS_LOADING) return "Loading"; if(app->status==ISS_STATUS_ERROR) return "Error"; return "Offline"; }
static const char *light_name(IssTrackerApp *app){ if(app->language==ISS_LANG_DE){ if(app->light_state==ISS_LIGHT_DAY) return "Tag"; if(app->light_state==ISS_LIGHT_TWILIGHT) return "Daemmerung"; return "Nacht"; } if(app->language==ISS_LANG_PL){ if(app->light_state==ISS_LIGHT_DAY) return "Dzien"; if(app->light_state==ISS_LIGHT_TWILIGHT) return "Zmierzch"; return "Noc"; } if(app->light_state==ISS_LIGHT_DAY) return "DAY"; if(app->light_state==ISS_LIGHT_TWILIGHT) return "TWILIGHT"; return "NIGHT"; }
static const char *surf_name(IssTrackerApp *app){ if(app->language==ISS_LANG_DE){ if(app->surface_state==ISS_SURFACE_LAND) return "Land"; if(app->surface_state==ISS_SURFACE_COAST) return "Kueste"; return "Wasser"; } if(app->language==ISS_LANG_PL){ if(app->surface_state==ISS_SURFACE_LAND) return "Lad"; if(app->surface_state==ISS_SURFACE_COAST) return "Wybrzeze"; return "Woda"; } if(app->surface_state==ISS_SURFACE_LAND) return "LAND"; if(app->surface_state==ISS_SURFACE_COAST) return "COAST"; return "WATER"; }
static const char *label_status(IssTrackerApp *app){ if(app->language==ISS_LANG_DE) return "Status: "; if(app->language==ISS_LANG_PL) return "Status: "; return "Status: "; }
static const char *label_daytime(IssTrackerApp *app){ if(app->language==ISS_LANG_DE) return "  *  Tageszeit: "; if(app->language==ISS_LANG_PL) return "  *  Pora dnia: "; return "  *  Daytime: "; }
static const char *label_terrain(IssTrackerApp *app){ if(app->language==ISS_LANG_DE) return "  *  Grund: "; if(app->language==ISS_LANG_PL) return "  *  Teren: "; return "  *  Terrain: "; }

static UBYTE max_pen_for_depth(UBYTE depth){ if(depth>=8) return 255; if(depth>=1) return (UBYTE)((1U<<depth)-1U); return 1; }
static UBYTE use_pen(IssTrackerApp *app, UBYTE depth, UBYTE pen){ UBYTE maxp; (void)app; maxp=max_pen_for_depth(depth); if(pen>maxp) return maxp; return pen; }
static UBYTE contrast_pen(UBYTE depth, UBYTE bg){ UBYTE maxp; maxp=max_pen_for_depth(depth); if(bg!=0) return 0; if(maxp>=1) return 1; return 0; }
static UBYTE text_pen(IssTrackerApp *app, UBYTE depth){ UBYTE fg; UBYTE bg; fg=use_pen(app,depth,app->pen_text); bg=use_pen(app,depth,app->pen_panel_bg); if(fg==bg) fg=contrast_pen(depth,bg); return fg; }
static UBYTE visible_map_pen(IssTrackerApp *app, UBYTE depth, UBYTE src_depth, UBYTE src)
{
    UBYTE maxp;
    UBYTE water;
    UBYTE land;
    UBYTE pen;
    maxp=max_pen_for_depth(depth);
    water=use_pen(app,depth,app->pen_water);
    land=use_pen(app,depth,app->pen_land);
    if(water==land && maxp>=2){
        water=1;
        land=2;
    } else if(water==land && maxp==1){
        water=0;
        land=1;
    }
    if(src_depth<4) pen=(src==1)?water:land;
    else pen=(src==2)?water:land;
    return pen;
}

static void layout(struct Window *win, WORD *mx, WORD *my, WORD *mw, WORD *mh, WORD *by, WORD *sy, WORD *iy){ WORD w; WORD h; w=(WORD)(win->Width-win->BorderLeft-win->BorderRight); h=(WORD)(win->Height-win->BorderTop-win->BorderBottom); *mx=(WORD)(win->BorderLeft+PAD_X); *my=(WORD)(win->BorderTop+MAP_TOP); *mw=(WORD)(w-(PAD_X*2)-6); if(*mw<MIN_MAP_W) *mw=MIN_MAP_W; *mh=(WORD)((LONG)(*mw)*ISS_MAP_SRC_H/ISS_MAP_SRC_W); if(*mh>(WORD)(h-104)) *mh=(WORD)(h-104); if(*mh<MIN_MAP_H) *mh=MIN_MAP_H; *by=(WORD)(*my+*mh+8); *sy=(WORD)(*by+BTN_H+STATUS_GAP); *iy=(WORD)(*sy+16); }
static void append_num(char *b, UWORD *p, LONG n){ char t[12]; WORD i; WORD j; if(n<0){ b[(*p)++]='-'; n=-n; } i=0; do{ t[i++]=(char)('0'+(n%10)); n/=10; }while(n&&i<11); for(j=i-1;j>=0;j--) b[(*p)++]=t[j]; b[*p]=0; }
static void append_str(char *b, UWORD *p, const char *s){ while(*s && *p<118) b[(*p)++]=*s++; b[*p]=0; }
static UBYTE is_leap_year(UWORD y){ return ((y%4)==0 && (((y%100)!=0) || ((y%400)==0))) ? 1 : 0; }
static void append_2digit(char *b, UWORD *p, UWORD n){ b[(*p)++]=(char)('0'+((n/10)%10)); b[(*p)++]=(char)('0'+(n%10)); b[*p]=0; }
static void append_4digit(char *b, UWORD *p, UWORD n){ b[(*p)++]=(char)('0'+((n/1000)%10)); b[(*p)++]=(char)('0'+((n/100)%10)); b[(*p)++]=(char)('0'+((n/10)%10)); b[(*p)++]=(char)('0'+(n%10)); b[*p]=0; }
static void append_utc_time(char *b, UWORD *p, ULONG ts){ static const UBYTE mdays[12]={31,28,31,30,31,30,31,31,30,31,30,31}; ULONG days; ULONG sod; UWORD y; UWORD m; UWORD d; UWORD dim; days=ts/86400UL; sod=ts%86400UL; y=1970; while(1){ d=is_leap_year(y)?366:365; if(days<d) break; days-=d; y++; } for(m=0;m<12;m++){ dim=mdays[m]; if(m==1 && is_leap_year(y)) dim=29; if(days<dim) break; days-=dim; } append_4digit(b,p,y); b[(*p)++]='-'; append_2digit(b,p,(UWORD)(m+1)); b[(*p)++]='-'; append_2digit(b,p,(UWORD)(days+1)); b[(*p)++]=' '; append_2digit(b,p,(UWORD)(sod/3600UL)); b[(*p)++]=':'; append_2digit(b,p,(UWORD)((sod/60UL)%60UL)); append_str(b,p," UTC"); }
static ULONG orbit_number(ULONG ts){ return (ULONG)(ts/5576UL); }
static UBYTE near_europe(IssTrackerApp *app){ if(!app->current.valid) return 0; if(app->current.lat_cd>=3500 && app->current.lat_cd<=7200 && app->current.lon_cd>=-2500 && app->current.lon_cd<=4500) return 2; if(app->current.lat_cd>=2500 && app->current.lat_cd<=7500 && app->current.lon_cd>=-6000 && app->current.lon_cd<=5500) return 1; return 0; }
static const char *visibility_name(IssTrackerApp *app){ if(app->visibility==0){ if(app->language==ISS_LANG_PL) return "dzien"; if(app->language==ISS_LANG_DE) return "Tag"; return "daylight"; } if(app->visibility==1){ if(app->language==ISS_LANG_PL) return "cien"; if(app->language==ISS_LANG_DE) return "Schatten"; return "eclipsed"; } if(app->language==ISS_LANG_PL) return "nieznane"; if(app->language==ISS_LANG_DE) return "unbekannt"; return "unknown"; }
static void append_crew_summary(IssTrackerApp *app,char *buf,UWORD *p){ UWORD i; UWORD iss; UWORD other; UWORD total; iss=0; other=0; for(i=0;i<app->crew_count;i++){ if(strcmp(app->crew[i].craft,"ISS")==0) iss++; else other++; } total=app->people_in_space; if(total==0 || total>64 || total<app->crew_count) total=app->crew_count; if(app->language==ISS_LANG_PL) append_str(buf,p,"Ludzie w kosmosie: "); else if(app->language==ISS_LANG_DE) append_str(buf,p,"Menschen im All: "); else append_str(buf,p,"People in space: "); append_num(buf,p,total); append_str(buf,p,"  *  ISS: "); append_num(buf,p,iss); if(other){ if(app->language==ISS_LANG_DE) append_str(buf,p,"  *  Andere: "); else if(app->language==ISS_LANG_PL) append_str(buf,p,"  *  Inne: "); else append_str(buf,p,"  *  Other: "); append_num(buf,p,other); } }
static void append_crew_names(IssTrackerApp *app,char *buf,UWORD *p){ UWORD i; if(app->crew_count==0){ if(app->language==ISS_LANG_PL) append_str(buf,p,"Lista zalogi niedostepna"); else if(app->language==ISS_LANG_DE) append_str(buf,p,"Crewliste nicht verfuegbar"); else append_str(buf,p,"Crew list unavailable"); return; } if(app->language==ISS_LANG_PL) append_str(buf,p,"Zaloga: "); else if(app->language==ISS_LANG_DE) append_str(buf,p,"Crew: "); else append_str(buf,p,"Crew: "); for(i=0;i<app->crew_count && *p<112;i++){ if(i) append_str(buf,p,", "); append_str(buf,p,app->crew[i].name); append_str(buf,p," ("); append_str(buf,p,app->crew[i].craft); append_str(buf,p,")"); } }
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
    if(app->status_page==1){
        if(app->language==ISS_LANG_PL) append_str(buf,&p,"Pozycja: Lat "); else append_str(buf,&p,"Position: Lat ");
        append_num(buf,&p,app->current.lat_cd/100); append_str(buf,&p,"  Lon "); append_num(buf,&p,app->current.lon_cd/100); return;
    }
    if(app->status_page==2){
        if(app->enhanced_valid){
            if(app->language==ISS_LANG_EN) append_str(buf,&p,"ISS: Altitude "); else if(app->language==ISS_LANG_PL) append_str(buf,&p,"ISS: Wysokosc "); else append_str(buf,&p,"ISS: Hoehe ");
            append_num(buf,&p,app->altitude_km); append_str(buf,&p," km  *  ");
            if(app->language==ISS_LANG_EN) append_str(buf,&p,"Speed "); else if(app->language==ISS_LANG_PL) append_str(buf,&p,"Predkosc "); else append_str(buf,&p,"Tempo ");
            append_num(buf,&p,app->velocity_kmh); append_str(buf,&p," km/h  *  ");
            if(app->language==ISS_LANG_EN) append_str(buf,&p,"Visibility "); else if(app->language==ISS_LANG_PL) append_str(buf,&p,"Widocznosc "); else append_str(buf,&p,"Sicht ");
            append_str(buf,&p,visibility_name(app));
        } else {
            if(app->language==ISS_LANG_EN) append_str(buf,&p,"ISS: Altitude 420 km  *  Speed 27600 km/h  *  Orbit "); else if(app->language==ISS_LANG_PL) append_str(buf,&p,"ISS: Wysokosc 420 km  *  Predkosc 27600 km/h  *  Orbita "); else append_str(buf,&p,"ISS: Hoehe 420 km  *  Tempo 27600 km/h  *  Orbit ");
            append_num(buf,&p,(LONG)orbit_number(app->current.timestamp));
        }
        return;
    }
    if(app->status_page==3){
        if(app->enhanced_valid){
            if(app->language==ISS_LANG_EN) append_str(buf,&p,"Footprint "); else if(app->language==ISS_LANG_PL) append_str(buf,&p,"Zasieg "); else append_str(buf,&p,"Footprint ");
            append_num(buf,&p,app->footprint_km); append_str(buf,&p," km  *  Solar Lat ");
            append_num(buf,&p,app->solar_lat_cd/100); append_str(buf,&p," Lon "); append_num(buf,&p,app->solar_lon_cd/100);
        } else {
            if(app->language==ISS_LANG_EN) append_str(buf,&p,"Footprint 4500 km  *  Solar Lat 0 Lon 0");
            else if(app->language==ISS_LANG_PL) append_str(buf,&p,"Zasieg 4500 km  *  Slonce Lat 0 Lon 0");
            else append_str(buf,&p,"Footprint 4500 km  *  Sonne Lat 0 Lon 0");
        }
        return;
    }
    if(app->status_page==4){
        europe=near_europe(app);
        if(app->language==ISS_LANG_EN){ if(europe==2) append_str(buf,&p,"Notice: ISS is over Europe  *  "); append_str(buf,&p,"Updated "); }
        else if(app->language==ISS_LANG_PL){ if(europe==2) append_str(buf,&p,"Info: ISS jest nad Europa  *  "); append_str(buf,&p,"Aktualizacja "); }
        else { if(europe==2) append_str(buf,&p,"Hinweis: ISS ist ueber Europa  *  "); append_str(buf,&p,"Aktualisiert "); }
        append_utc_time(buf,&p,app->current.timestamp);
        return;
    }
    if(app->status_page==5){ append_crew_summary(app,buf,&p); return; }
    append_crew_names(app,buf,&p);
}
static UWORD split_text_to_width(struct RastPort *rp, const char *s, WORD max_width)
{
    UWORD i;
    UWORD last_space;
    UWORD len;
    len=(UWORD)strlen(s);
    last_space=0;
    for(i=1;i<=len;i++){
        if(s[i-1]==' ') last_space=i-1;
        if(TextLength(rp,(STRPTR)s,i)>max_width){
            if(last_space>0) return last_space;
            return (i>1) ? (UWORD)(i-1) : 1;
        }
    }
    return len;
}
static void draw_wrapped_lines(struct RastPort *rp, WORD x, WORD y, WORD max_width, const char *s, UWORD max_lines)
{
    UWORD split;
    UWORD line;
    line=0;
    while(*s && line<max_lines){
        while(*s==' ') s++;
        if(!*s) break;
        split=split_text_to_width(rp,s,max_width);
        text_at_n(rp,x,(WORD)(y+(WORD)(line*STATUS_GAP)),s,split);
        s+=split;
        line++;
    }
}
static void draw_crew_lines(struct RastPort *rp, IssTrackerApp *app, WORD x, WORD y, WORD max_width)
{
    char line[120];
    char entry[64];
    UWORD p;
    UWORD ep;
    UWORD i;
    UWORD row;
    if(app->crew_count==0){
        p=0;
        if(app->language==ISS_LANG_PL) append_str(line,&p,"Lista zalogi niedostepna");
        else if(app->language==ISS_LANG_DE) append_str(line,&p,"Crewliste nicht verfuegbar");
        else append_str(line,&p,"Crew list unavailable");
        text_at(rp,x,y,line);
        return;
    }
    row=0;
    p=0;
    if(app->language==ISS_LANG_PL) append_str(line,&p,"Zaloga: ");
    else append_str(line,&p,"Crew: ");
    for(i=0;i<app->crew_count && row<4;i++){
        ep=0;
        if(p>0 && line[p-1]!=' ') append_str(entry,&ep,", ");
        append_str(entry,&ep,app->crew[i].name);
        append_str(entry,&ep," (");
        append_str(entry,&ep,app->crew[i].craft);
        append_str(entry,&ep,")");
        if(p>0 && TextLength(rp,(STRPTR)line,p)+TextLength(rp,(STRPTR)entry,ep)>max_width){
            text_at(rp,x,(WORD)(y+(WORD)(row*STATUS_GAP)),line);
            row++;
            if(row>=4) break;
            p=0;
            append_str(line,&p,app->crew[i].name);
            append_str(line,&p," (");
            append_str(line,&p,app->crew[i].craft);
            append_str(line,&p,")");
        } else {
            append_str(line,&p,entry);
        }
    }
    if(row<4 && p>0) text_at(rp,x,(WORD)(y+(WORD)(row*STATUS_GAP)),line);
}
static void set_button_visible_pen(struct Gadget *g, UBYTE pen)
{
    if(!g) return;
    if(g->GadgetText) ((struct IntuiText *)g->GadgetText)->FrontPen=pen;
    if(g->GadgetRender) ((struct Border *)g->GadgetRender)->FrontPen=pen;
}
static struct TextAttr button_topaz8_attr = { (STRPTR)"topaz.font", 8, FS_NORMAL, FPF_ROMFONT };
static struct TextFont *button_topaz8_font(void)
{
    static struct TextFont *font = 0;
    if(!font) font = OpenFont(&button_topaz8_attr);
    return font;
}

static void draw_main_button(struct RastPort *rp, WORD x, WORD y, WORD w, WORD h, const char *label, UBYTE bgpen, UBYTE fgpen)
{
    struct TextFont *old_font;
    struct TextFont *btn_font;
    old_font=rp->Font;
    btn_font=button_topaz8_font();
    if(btn_font) SetFont(rp,btn_font);
    SetDrMd(rp,JAM1);
    SetAPen(rp,bgpen);
    RectFill(rp,x,y,(WORD)(x+w-1),(WORD)(y+h-1));
    SetAPen(rp,fgpen);
    Move(rp,x,y); Draw(rp,(WORD)(x+w-1),y); Draw(rp,(WORD)(x+w-1),(WORD)(y+h-1)); Draw(rp,x,(WORD)(y+h-1)); Draw(rp,x,y);
    Move(rp,(WORD)(x+6),(WORD)(y+11));
    Text(rp,(STRPTR)label,strlen(label));
    if(old_font) SetFont(rp,old_font);
}
static void draw_map_rect(struct RastPort *rp, IssTrackerApp *app, UBYTE depth, WORD map_x, WORD map_y, WORD map_w, WORD map_h, WORD rx1, WORD ry1, WORD rx2, WORD ry2){ WORD y; WORD x; WORD sx; WORD sy; WORD run_start; UBYTE pen; UBYTE last; UBYTE src_depth; if(rx1<0) rx1=0; if(ry1<0) ry1=0; if(rx2>=map_w) rx2=(WORD)(map_w-1); if(ry2>=map_h) ry2=(WORD)(map_h-1); if(rx1>rx2 || ry1>ry2) return; src_depth=(depth<4)?2:4; for(y=ry1;y<=ry2;y++){ sy=(WORD)(((LONG)y*ISS_MAP_SRC_H)/map_h); run_start=rx1; sx=(WORD)(((LONG)rx1*ISS_MAP_SRC_W)/map_w); last=visible_map_pen(app,depth,src_depth,iss_map_get_pen(src_depth,sx,sy)); for(x=(WORD)(rx1+1);x<=rx2;x++){ sx=(WORD)(((LONG)x*ISS_MAP_SRC_W)/map_w); pen=visible_map_pen(app,depth,src_depth,iss_map_get_pen(src_depth,sx,sy)); if(pen!=last){ SetAPen(rp,last); Move(rp,map_x+run_start,map_y+y); Draw(rp,map_x+x-1,map_y+y); run_start=x; last=pen; } } SetAPen(rp,last); Move(rp,map_x+run_start,map_y+y); Draw(rp,map_x+rx2,map_y+y); } }
static void draw_map(struct RastPort *rp, IssTrackerApp *app, UBYTE depth, WORD map_x, WORD map_y, WORD map_w, WORD map_h){ draw_map_rect(rp,app,depth,map_x,map_y,map_w,map_h,0,0,(WORD)(map_w-1),(WORD)(map_h-1)); SetAPen(rp,use_pen(app,depth,app->pen_border)); Move(rp,map_x,map_y); Draw(rp,map_x+map_w,map_y); Draw(rp,map_x+map_w,map_y+map_h); Draw(rp,map_x,map_y+map_h); Draw(rp,map_x,map_y); }
static void draw_marker_shape(struct RastPort *rp, IssTrackerApp *app, UBYTE depth, WORD px, WORD py){ SetAPen(rp,use_pen(app,depth,app->pen_marker)); Move(rp,px-5,py); Draw(rp,px+5,py); Move(rp,px,py-5); Draw(rp,px,py+5); SetAPen(rp,use_pen(app,depth,app->pen_marker_fill)); RectFill(rp,px-2,py-2,px+2,py+2); }
static void draw_trail_clip(struct RastPort *rp, IssTrackerApp *app, UBYTE depth, WORD map_x, WORD map_y, WORD map_w, WORD map_h, WORD rx1, WORD ry1, WORD rx2, WORD ry2){ WORD px; WORD py; WORD lx; WORD ly; UWORD i; UWORD idx; if(depth<4 && !app->show_trail) return; SetAPen(rp,use_pen(app,depth,app->pen_trail)); for(i=0;i<app->trail_count;i++){ idx=(app->trail_head+ISS_TRAIL_MAX-app->trail_count+i)%ISS_TRAIL_MAX; if(app->trail[idx].valid){ iss_project_equirect(app->trail[idx].lat_cd,app->trail[idx].lon_cd,map_x,map_y,map_w,map_h,&px,&py); lx=(WORD)(px-map_x); ly=(WORD)(py-map_y); if(lx>=rx1-1 && lx<=rx2+1 && ly>=ry1-1 && ly<=ry2+1) RectFill(rp,px-1,py-1,px+1,py+1); } } }
static void draw_iss(struct RastPort *rp,IssTrackerApp *app, UBYTE depth, WORD map_x, WORD map_y, WORD map_w, WORD map_h){ WORD px; WORD py; draw_trail_clip(rp,app,depth,map_x,map_y,map_w,map_h,0,0,(WORD)(map_w-1),(WORD)(map_h-1)); if(app->current.valid && app->blink){ iss_project_equirect(app->current.lat_cd,app->current.lon_cd,map_x,map_y,map_w,map_h,&px,&py); draw_marker_shape(rp,app,depth,px,py); } }
void draw_iss_blink(struct Window *win, IssTrackerApp *app){ struct RastPort *rp; UBYTE depth; WORD mx; WORD my; WORD mw; WORD mh; WORD by; WORD sy; WORD iy; WORD px; WORD py; WORD lx; WORD ly; WORD rx1; WORD ry1; WORD rx2; WORD ry2; if(!app->current.valid) return; rp=win->RPort; if(app->ui_font) SetFont(rp,app->ui_font); depth=(UBYTE)win->WScreen->BitMap.Depth; layout(win,&mx,&my,&mw,&mh,&by,&sy,&iy); iss_project_equirect(app->current.lat_cd,app->current.lon_cd,mx,my,mw,mh,&px,&py); lx=(WORD)(px-mx); ly=(WORD)(py-my); rx1=(WORD)(lx-MARK_R); ry1=(WORD)(ly-MARK_R); rx2=(WORD)(lx+MARK_R); ry2=(WORD)(ly+MARK_R); SetDrMd(rp,JAM1); draw_map_rect(rp,app,depth,mx,my,mw,mh,rx1,ry1,rx2,ry2); draw_trail_clip(rp,app,depth,mx,my,mw,mh,rx1,ry1,rx2,ry2); if(app->blink) draw_marker_shape(rp,app,depth,px,py); }
void draw_panel(struct Window *win, IssTrackerApp *app){ struct RastPort *rp; WORD mx; WORD my; WORD mw; WORD mh; WORD by; WORD sy; WORD iy; WORD right; WORD bottom; char line[120]; rp=win->RPort; if(app->ui_font) SetFont(rp,app->ui_font); layout(win,&mx,&my,&mw,&mh,&by,&sy,&iy); right=(WORD)(win->Width-win->BorderRight-1); bottom=(WORD)(win->Height-win->BorderBottom-1); SetDrMd(rp,JAM1); SetAPen(rp,use_pen(app,(UBYTE)win->WScreen->BitMap.Depth,app->pen_panel_bg)); if(app->update_gadget && app->info_gadget) RectFill(rp,win->BorderLeft,(WORD)(by+BTN_H+1),right,bottom); else RectFill(rp,win->BorderLeft,by-2,right,bottom); if(app->update_gadget && app->info_gadget){ UBYTE bpen; UBYTE bgpen; bpen=text_pen(app,(UBYTE)win->WScreen->BitMap.Depth); bgpen=use_pen(app,(UBYTE)win->WScreen->BitMap.Depth,app->pen_panel_bg); app->update_gadget->LeftEdge=mx; app->update_gadget->TopEdge=by; app->info_gadget->LeftEdge=(WORD)(mx+64); app->info_gadget->TopEdge=by; set_button_visible_pen(app->update_gadget,bpen); set_button_visible_pen(app->info_gadget,bpen); draw_main_button(rp,mx,by,BTN_W,BTN_H,"Update",bgpen,bpen); draw_main_button(rp,(WORD)(mx+64),by,BTN_W,BTN_H,"Info",bgpen,bpen); } if(app->ui_font) SetFont(rp,app->ui_font); SetAPen(rp,text_pen(app,(UBYTE)win->WScreen->BitMap.Depth)); make_status_line(app,line); text_at(rp,mx,sy,line); if(!app->funfact_active && app->status_page==6 && app->current.valid) draw_crew_lines(rp,app,mx,iy,(WORD)(right-mx-4)); else { make_detail_line(app,line); if(app->funfact_active) draw_wrapped_lines(rp,mx,iy,(WORD)(right-mx-4),line,3); else text_at(rp,mx,iy,line); } }
void draw_all(struct Window *win, IssTrackerApp *app){ struct RastPort *rp; UBYTE depth; WORD mx; WORD my; WORD mw; WORD mh; WORD by; WORD sy; WORD iy; WORD right; WORD bottom; rp=win->RPort; if(app->ui_font) SetFont(rp,app->ui_font); depth=(UBYTE)win->WScreen->BitMap.Depth; layout(win,&mx,&my,&mw,&mh,&by,&sy,&iy); right=(WORD)(win->Width-win->BorderRight-1); bottom=(WORD)(win->Height-win->BorderBottom-1); SetDrMd(rp,JAM1); SetAPen(rp,use_pen(app,depth,app->pen_panel_bg)); RectFill(rp,win->BorderLeft,win->BorderTop,right,bottom); draw_map(rp,app,depth,mx,my,mw,mh); draw_iss(rp,app,depth,mx,my,mw,mh); draw_panel(win,app); }
