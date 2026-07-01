#ifndef ISS_API_H
#define ISS_API_H
#include "app.h"
LONG iss_fetch_now(IssTrackerApp *app, IssPosition *pos, char *err, UWORD err_size);
LONG iss_fetch_crew(IssTrackerApp *app, char *err, UWORD err_size);
UBYTE iss_https_probe(IssTrackerApp *app);
void iss_https_shutdown(void);
#endif
