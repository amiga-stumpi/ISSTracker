#ifndef ISS_DRAW_H
#define ISS_DRAW_H
#include <intuition/intuition.h>
#include "app.h"
void draw_all(struct Window *win, IssTrackerApp *app);
void draw_panel(struct Window *win, IssTrackerApp *app);
void draw_iss_blink(struct Window *win, IssTrackerApp *app);
#endif
