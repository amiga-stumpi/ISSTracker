#ifndef ISS_APP_H
#define ISS_APP_H
#include <exec/types.h>
#include <graphics/text.h>
struct Gadget;
#define ISS_VERSION "ISSTracker live v1.3.0"
#define ISS_TITLE "ISSTracker live v1.3.0"
#define ISS_TRAIL_MAX 16
#define ISS_CREW_MAX 12
#define ISS_STATUS_OFFLINE 0
#define ISS_STATUS_LOADING 1
#define ISS_STATUS_ONLINE 2
#define ISS_STATUS_ERROR 3
#define ISS_LIGHT_DAY 0
#define ISS_LIGHT_TWILIGHT 1
#define ISS_LIGHT_NIGHT 2
#define ISS_SURFACE_WATER 0
#define ISS_SURFACE_LAND 1
#define ISS_SURFACE_COAST 2
#define ISS_LANG_DE 0
#define ISS_LANG_EN 1
#define ISS_LANG_PL 2
typedef struct IssPosition { WORD lat_cd; WORD lon_cd; ULONG timestamp; UBYTE valid; } IssPosition;
typedef struct IssCrewMember { char name[32]; char craft[16]; } IssCrewMember;
typedef struct IssTrackerApp { IssPosition current; IssPosition trail[ISS_TRAIL_MAX]; UWORD trail_head; UWORD trail_count; UBYTE show_trail; UBYTE auto_update; UBYTE blink; UBYTE status; UBYTE light_state; UBYTE surface_state; UBYTE status_page; UBYTE language; UBYTE funfact_active; UBYTE europe_alert_seen; UWORD funfact_index; UWORD funfact_ticks; UWORD funfact_next_ticks; ULONG funfact_seed; char status_text[80]; char info_text[128]; WORD win_left; WORD win_top; WORD win_width; WORD win_height; UWORD update_interval_min; UBYTE pen_water; UBYTE pen_land; UBYTE pen_border; UBYTE pen_marker; UBYTE pen_marker_fill; UBYTE pen_trail; UBYTE pen_text; UBYTE pen_panel_bg; char font_name[32]; UWORD font_size; struct TextFont *ui_font; struct Gadget *update_gadget; struct Gadget *info_gadget; UBYTE https_available; UBYTE enhanced_valid; UBYTE visibility; UWORD altitude_km; UWORD velocity_kmh; UWORD footprint_km; WORD solar_lat_cd; WORD solar_lon_cd; UWORD people_in_space; UWORD crew_count; UBYTE crew_fetch_state; IssCrewMember crew[ISS_CREW_MAX]; } IssTrackerApp;
void iss_app_add_position(IssTrackerApp *app, const IssPosition *pos);
#endif
