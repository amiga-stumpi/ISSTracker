#ifndef ISS_APP_H
#define ISS_APP_H
#include <exec/types.h>
#define ISS_VERSION "ISS Tracker v1.0"
#define ISS_TITLE "ISS Tracker v1.0"
#define ISS_TRAIL_MAX 16
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
typedef struct IssPosition { WORD lat_cd; WORD lon_cd; ULONG timestamp; UBYTE valid; } IssPosition;
typedef struct IssTrackerApp { IssPosition current; IssPosition trail[ISS_TRAIL_MAX]; UWORD trail_head; UWORD trail_count; UBYTE show_trail; UBYTE auto_update; UBYTE blink; UBYTE status; UBYTE light_state; UBYTE surface_state; char status_text[80]; char info_text[128]; WORD win_left; WORD win_top; WORD win_width; WORD win_height; } IssTrackerApp;
void iss_app_add_position(IssTrackerApp *app, const IssPosition *pos);
#endif
