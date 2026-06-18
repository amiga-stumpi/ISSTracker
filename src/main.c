#include <exec/types.h>
#include <exec/libraries.h>
#include <proto/exec.h>
#include <string.h>
#include "app.h"
#include "gui.h"
struct Library *IntuitionBase = 0;
struct Library *GfxBase = 0;
struct Library *SocketBase = 0;
void iss_app_add_position(IssTrackerApp *app, const IssPosition *pos){ app->trail[app->trail_head]=*pos; app->trail_head=(UWORD)((app->trail_head+1)%ISS_TRAIL_MAX); if(app->trail_count<ISS_TRAIL_MAX) app->trail_count++; }
int main(void){ IssTrackerApp app; memset(&app,0,sizeof(app)); app.show_trail=1; app.status=ISS_STATUS_OFFLINE; app.blink=1; strcpy(app.info_text,"Press Update to fetch ISS position"); IntuitionBase=OpenLibrary("intuition.library",0); GfxBase=OpenLibrary("graphics.library",0); SocketBase=OpenLibrary("bsdsocket.library",4); if(!IntuitionBase || !GfxBase){ if(SocketBase) CloseLibrary(SocketBase); if(GfxBase) CloseLibrary(GfxBase); if(IntuitionBase) CloseLibrary(IntuitionBase); return 20; } gui_run(&app); if(SocketBase) CloseLibrary(SocketBase); if(GfxBase) CloseLibrary(GfxBase); if(IntuitionBase) CloseLibrary(IntuitionBase); return 0; }
